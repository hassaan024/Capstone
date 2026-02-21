import { Injectable, Logger } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';
import { DatabaseService } from 'database/database.service';
import { GoogleGenerativeAI } from '@google/generative-ai';

interface ChatMessage {
  role: 'user' | 'model';
  parts: { text: string }[];
}

@Injectable()
export class ChatService {
  private readonly logger = new Logger(ChatService.name);
  private genAI: GoogleGenerativeAI | null = null;

  constructor(
    private readonly configService: ConfigService,
    private readonly db: DatabaseService,
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
   * Build a personalized system prompt that includes the user's saved plants.
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
        }
      } catch (err) {
        this.logger.warn(`Failed to fetch user context: ${(err as Error).message}`);
      }
    }

    return `You are "Leafy", a friendly and knowledgeable plant care assistant for the app "LeafyLedger". 
You help users with:
- Plant care advice (watering, sunlight, soil, fertilizing)
- Diagnosing plant health problems
- Recommending plants based on their experience level, location, or preferences
- Companion planting and garden layout tips
- Seasonal gardening advice

Guidelines:
- Be warm, encouraging, and enthusiastic about plants 🌿
- Keep responses concise but helpful (2-3 paragraphs max unless the user asks for detail)
- Use emoji sparingly to keep it friendly
- If you don't know something specific, say so honestly
- When relevant, reference the user's saved plants or gardens to personalize your advice
- Format responses with markdown: use **bold** for plant names and important terms, bullet lists for tips
${userContext}`;
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

      const model = this.genAI.getGenerativeModel({
        model: 'gemini-2.0-flash',
        systemInstruction: systemPrompt,
      });

      // Build chat history
      const chatHistory: ChatMessage[] = history || [];

      const chat = model.startChat({
        history: chatHistory,
      });

      const result = await chat.sendMessage(message);
      const response = result.response;
      const text = response.text();

      return { reply: text };
    } catch (err: unknown) {
      const error = err as Error & { status?: number; message?: string };
      this.logger.error(`Gemini API error: ${error.message}`);

      // Handle rate limit / quota errors
      if (
        error.status === 429 ||
        error.message?.includes('quota') ||
        error.message?.includes('RATE_LIMIT') ||
        error.message?.includes('RESOURCE_EXHAUSTED')
      ) {
        return {
          error: 'quota_exceeded',
          message:
            'Daily token limit has been reached. Please try again tomorrow — the free tier resets every 24 hours! 🌱',
        };
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

      return {
        error: 'api_error',
        message:
          'Something went wrong connecting to the AI. Please try again in a moment.',
      };
    }
  }
}
