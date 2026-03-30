import { Injectable, Logger } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';
import { DatabaseService } from 'database/database.service';
import { GoogleGenerativeAI } from '@google/generative-ai';
import { WeatherService } from '../weather/weather.service';

interface ChatMessage {
  role: 'user' | 'model';
  parts: { text: string }[];
}

@Injectable()
export class ChatService {
  private readonly logger = new Logger(ChatService.name);
  private genAI: GoogleGenerativeAI | null = null;

  /** Models to try in order — if one hits quota, fall back to the next */
  private readonly MODELS = ['gemini-2.5-flash', 'gemini-2.5-flash-lite'];

  constructor(
    private readonly configService: ConfigService,
    private readonly db: DatabaseService,
    private readonly weatherService: WeatherService,
  ) {
    const apiKey =
      this.configService.get<string>('GEMINI_API_KEY') ||
      process.env.GEMINI_API_KEY ||
      '';

    if (!apiKey) {
      this.logger.warn(
        'GEMINI_API_KEY is not set! Chat feature will be unavailable.',
      );
    } else {
      this.genAI = new GoogleGenerativeAI(apiKey);
    }
  }

  /**
   * Build a personalized system prompt that includes the user's saved plants,
   * live weather data, and species-vs-weather alerts.
   */
  private async buildSystemPrompt(userId?: number): Promise<string> {
    let userContext = '';

    if (userId) {
      try {
        const user = await this.db.user.findUnique({
          where: { id: userId },
          include: {
            savedSpecies: true,
            gardens: {
              include: {
                plants: {
                  include: { species: true },
                },
              },
            },
          },
        });

        if (user) {
          userContext += `\n\nThe user's name is "${user.displayName}".`;

          if (user.savedSpecies && user.savedSpecies.length > 0) {
            const plantList = user.savedSpecies
              .map(
                (s) =>
                  `- ${s.commonName} (${s.scientificName})${s.wateringFreq ? `, watering: ${s.wateringFreq}` : ''}`,
              )
              .join('\n');
            userContext += `\n\nThe user has saved these plants in their collection:\n${plantList}`;
          }

          if (user.gardens && user.gardens.length > 0) {
            const gardenInfo = user.gardens
              .map((g) => {
                const plantCount = g.plants?.length || 0;
                return `- "${g.name}" (${plantCount} plants, lat: ${g.latitude}, lng: ${g.longitude})`;
              })
              .join('\n');
            userContext += `\n\nThe user's gardens:\n${gardenInfo}`;
          }

          // --- Live weather context ---
          const typedUser = user as typeof user & {
            latitude?: number | null;
            longitude?: number | null;
          };
          const lat = typedUser.latitude ?? user.gardens?.[0]?.latitude;
          const lng = typedUser.longitude ?? user.gardens?.[0]?.longitude;

          if (
            lat !== undefined &&
            lat !== null &&
            lng !== undefined &&
            lng !== null
          ) {
            try {
              const weather = await this.weatherService.getCurrentDaysWeather(
                lat,
                lng,
              );

              userContext +=
                `\n\nCurrent weather at the user's location:` +
                `\n- Temperature: ${weather.temperature_2m}°F` +
                `\n- Conditions: ${weather.description}` +
                `\n- Humidity: ${weather.relative_humidity_2m}%` +
                `\n- Wind: ${weather.wind_speed_10m} mph` +
                `\n- VPD (vapor pressure deficit): ${weather.vapour_pressure_deficit} kPa` +
                `\n- Daily evaporation (ET0): ${weather.daily_evaporation} mm` +
                `\n- Sunlight intensity: ${weather.sunlight_intensity} MJ/m²`;

              // --- Species-vs-weather alerts ---
              // Collect all unique species from gardens and saved species
              const speciesMap = new Map<
                number,
                {
                  commonName: string;
                  minTemp?: number | null;
                  maxTemp?: number | null;
                  minHumidity?: number | null;
                  maxHumidity?: number | null;
                }
              >();

              if (user.savedSpecies) {
                for (const s of user.savedSpecies) {
                  speciesMap.set(s.id, s);
                }
              }
              if (user.gardens) {
                for (const g of user.gardens) {
                  if (g.plants) {
                    for (const p of g.plants) {
                      if (p.species) {
                        speciesMap.set(p.species.id, p.species);
                      }
                    }
                  }
                }
              }

              const alerts: string[] = [];
              const currentTemp = weather.temperature_2m ?? 0;
              const currentHumidity = weather.relative_humidity_2m ?? 0;

              for (const species of speciesMap.values()) {
                // Temperature alerts (species thresholds are stored in °F based on the dto)
                if (species.minTemp != null && currentTemp < species.minTemp) {
                  alerts.push(
                    `⚠️ ${species.commonName}: Current temp (${currentTemp}°F) is BELOW its minimum preferred temp (${species.minTemp}°F). Warn the user about cold stress.`,
                  );
                }
                if (species.maxTemp != null && currentTemp > species.maxTemp) {
                  alerts.push(
                    `⚠️ ${species.commonName}: Current temp (${currentTemp}°F) is ABOVE its maximum preferred temp (${species.maxTemp}°F). Warn the user about heat stress.`,
                  );
                }

                // Humidity alerts
                if (
                  species.minHumidity != null &&
                  currentHumidity < species.minHumidity
                ) {
                  alerts.push(
                    `⚠️ ${species.commonName}: Current humidity (${currentHumidity}%) is BELOW its minimum preferred humidity (${species.minHumidity}%). Suggest misting or a humidity tray.`,
                  );
                }
                if (
                  species.maxHumidity != null &&
                  currentHumidity > species.maxHumidity
                ) {
                  alerts.push(
                    `⚠️ ${species.commonName}: Current humidity (${currentHumidity}%) is ABOVE its maximum preferred humidity (${species.maxHumidity}%). Suggest improving airflow.`,
                  );
                }
              }

              if (alerts.length > 0) {
                userContext += `\n\nIMPORTANT — Plant condition alerts based on current weather:\n${alerts.join('\n')}`;
                userContext += `\nProactively mention these alerts when relevant. The user may not be aware of these conditions.`;
              }
            } catch (weatherErr) {
              this.logger.warn(
                `Failed to fetch weather for chat context: ${(weatherErr as Error).message}`,
              );
              // Weather fetch failed — continue without weather context
            }
          }
        }
      } catch (err) {
        this.logger.warn(
          `Failed to fetch user context: ${(err as Error).message}`,
        );
      }
    }

    return `You are "Leafy", a friendly and knowledgeable plant care assistant for the app "LeafyLedger". 
You help users with:
- Plant care advice (watering, sunlight, soil, fertilizing)
- Diagnosing plant health problems
- Recommending plants based on their experience level, location, or preferences
- Companion planting and garden layout tips
- Seasonal gardening advice
- Navigating and using the LeafyLedger app

Guidelines:
- **CRITICAL**: Be EXTREMELY concise and direct. Do NOT repeat the user's question. Get straight to the point without conversational filler.
- Limit responses to 2-3 short sentences or a brief bulleted list unless the user explicitly requests more detail.
- Be warm and encouraging, but prioritize brevity 🌿
- Use emoji sparingly to keep it friendly
- If you don't know something specific, say so honestly
- When relevant, reference the user's saved plants or gardens to personalize your advice
- If weather data is provided, use it to give climate-aware advice (e.g. watering needs, frost warnings, heat stress)
- If plant condition alerts are provided, proactively mention them to the user
- If the user asks how to do something in the app, guide them step-by-step using the App Guide below
- Format responses with markdown: use **bold** for plant names and important terms, bullet lists for tips

App Guide — How LeafyLedger Works:

1. LOGIN (/login):
   - The entry point to LeafyLedger. Users sign in to access their personalized data.
   - Supports secure authentication. Unauthenticated users are redirected here automatically.
   - After login, users land on the Dashboard.

2. DASHBOARD (/dashboard):
   - The main hub after logging in. Shows a welcome message, quick actions, and stats (gardens, plants, species counts).
   - Quick Actions: "Create New Garden", "View Saved Plants" → /saved-species, "Browse Species" → /browse.
   - Weather Widget: Shows live weather based on the user's location (temperature, humidity, forecast, plant stress metrics).
   - Location Setup: Users can enter a zip/postal code or use browser geolocation — saved to their profile.

3. BROWSE SPECIES (/browse):
   - A searchable database of plant species with images, descriptions, and care info.
   - Users can search by common name, scientific name, or keywords.
   - Click a species to see details. Click the bookmark icon to save it to their collection.

4. SAVED SPECIES (/saved-species):
   - A personal library of bookmarked plants from the Browse page.
   - Quick reference for care instructions without re-searching.
   - Users can remove species they no longer want to track.

5. CHATBOT (you!):
   - The floating chat widget available on every page — that's you, Leafy!
   - Users can expand/minimize the panel. Chat history is kept during the session.
   - You can help with plant care, weather-based advice, and navigating the app.
${userContext}`;
  }

  /**
   * Try to send a message using the given model name.
   * Returns the reply text on success, or throws on failure.
   */
  private async tryModel(
    modelName: string,
    systemPrompt: string,
    message: string,
    chatHistory: ChatMessage[],
  ): Promise<string> {
    const model = this.genAI!.getGenerativeModel({
      model: modelName,
      systemInstruction: systemPrompt,
    });

    const chat = model.startChat({ history: chatHistory });
    const result = await chat.sendMessage(message);
    return result.response.text();
  }

  async sendMessage(
    message: string,
    userId?: number,
    history?: ChatMessage[],
  ): Promise<{ reply: string } | { error: string; message: string }> {
    if (!this.genAI) {
      return {
        error: 'not_configured',
        message:
          'The chat feature is not configured. Please set the GEMINI_API_KEY.',
      };
    }

    try {
      const systemPrompt = await this.buildSystemPrompt(userId);
      const chatHistory: ChatMessage[] = history || [];

      // Try each model in order; fall back on quota/rate-limit errors
      let lastError: unknown = null;
      for (const modelName of this.MODELS) {
        try {
          this.logger.log(`Trying model: ${modelName}`);
          const text = await this.tryModel(
            modelName,
            systemPrompt,
            message,
            chatHistory,
          );
          return { reply: text };
        } catch (err: unknown) {
          const error = err as Error & { status?: number };
          // Only fall back on 429 (rate limit / quota) errors
          if (error.status === 429) {
            this.logger.warn(
              `Model ${modelName} quota exceeded, trying next fallback...`,
            );
            lastError = err;
            continue;
          }
          // For any other error, don't retry — throw immediately
          throw err;
        }
      }

      // All models exhausted their quota — throw the last error
      throw lastError;
    } catch (err: unknown) {
      const error = err as Error & {
        status?: number;
        message?: string;
        errorDetails?: Array<{ reason?: string; domain?: string }>;
      };
      this.logger.error(
        `Gemini API error: status=${error.status} message=${error.message}`,
      );
      if (error.errorDetails) {
        this.logger.error(
          `Gemini error details: ${JSON.stringify(error.errorDetails)}`,
        );
      }

      // Handle safety filters
      if (
        error.message?.includes('SAFETY') ||
        error.message?.includes('blocked')
      ) {
        return {
          error: 'safety_blocked',
          message:
            "I can only help with plant-related questions. Let's talk about gardening! 🌿",
        };
      }

      // Handle rate limit / quota errors — only match on HTTP 429
      // OR explicit RESOURCE_EXHAUSTED status (not just substring matches)
      if (error.status === 429) {
        return {
          error: 'quota_exceeded',
          message:
            'Daily token limit has been reached. Please try again tomorrow — the free tier resets every 24 hours! 🌱',
        };
      }

      // Handle API key issues (403 typically means invalid/revoked key)
      if (error.status === 403) {
        this.logger.error(
          'Gemini API returned 403 — the API key may be invalid or revoked. ' +
            'Please verify GEMINI_API_KEY in your .env file.',
        );
        return {
          error: 'api_key_invalid',
          message:
            'The chat service is temporarily unavailable. Please contact the administrator.',
        };
      }

      // Handle 400 errors (bad request, possibly invalid model or key)
      if (error.status === 400) {
        this.logger.error(
          'Gemini API returned 400 — check the model name and API key configuration.',
        );
        return {
          error: 'bad_request',
          message:
            'Something went wrong with the AI configuration. Please contact the administrator.',
        };
      }

      return {
        error: 'api_error',
        message:
          'Something went wrong connecting to the AI. Please try again in a moment.',
      };
    }
  }
}
