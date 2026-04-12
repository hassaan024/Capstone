import {
  Injectable,
  Logger,
  NotFoundException,
  BadRequestException,
} from '@nestjs/common';
import { DatabaseService } from 'database/database.service';
import { WeatherService } from 'weather/weather.service';
import { estimateSoilMoisture } from 'utils/soil-moisture';

@Injectable()
export class PredictionService {
  private readonly logger = new Logger(PredictionService.name);

  constructor(
    private readonly db: DatabaseService,
    private readonly weatherService: WeatherService,
  ) {}

  async predict(plantInstanceId: number, daysAhead: number) {
    // 1. Load plant + species + soil + garden
    const plant = await this.db.plantInstance.findUnique({
      where: { id: plantInstanceId },
      include: {
        species: true,
        soil: true,
        garden: true,
      },
    });

    if (!plant) throw new NotFoundException(`Plant instance ${plantInstanceId} not found`);

    const { species, soil, garden } = plant;

    // 2. Compute age from game dates
    const ageDays = Math.floor(
      (plant.currentGameDate.getTime() - plant.plantedDate.getTime()) / 86400000
    );

    // 3. Fetch weather anchored to the plant's current game date
    const weather = await this.weatherService.getWeatherForGameDate(
      garden.latitude,
      garden.longitude,
      plant.currentGameDate,
      Math.min(daysAhead, 30),
    );

    // 4. Estimate soil moisture from weather + soil type
    const soilMoisture = estimateSoilMoisture(weather, soil.type);

    // 5. Build payload for FastAPI
    const payload = {
      plant: {
        id: plant.id,
        heightCm: plant.heightCm ?? 0,
        ageDays,
        daysAhead,
        healthStatus: plant.healthStatus ?? null,
        growthStage: plant.growthStage ?? null,
        bloomState: plant.bloomState,
      },
      species: {
        growthRate: species.growthRate,
        minHeightCm: (species.minHeight ?? 0) * 30.48,
        maxHeightCm: (species.maxHeight ?? 100) * 30.48,
        avgHoursSun: species.avgHoursSun ?? null,
        minTemp: species.minTemp ?? null,
        maxTemp: species.maxTemp ?? null,
        wateringMinDays: species.wateringMinDays ?? null,
        wateringMaxDays: species.wateringMaxDays ?? null,
        droughtTolerant: species.droughtTolerant ?? false,
        tropical: species.tropical ?? false,
        cycle: species.cycle ?? null,
      },
      soil: {
        type: soil.type,
        pH: soil.pH,
        nitrogen: soil.nitrogen,
        phosphorus: soil.phosphorus,
        potassium: soil.potassium,
        organicPercentage: soil.organicPercentage ?? null,
        estimatedMoisture: soilMoisture,
      },
      weather: weather.map((day) => ({
        date: day.date,
        temperatureF: day.temperature_2m,
        precipitation: day.precipitation,
        sunlightIntensity: day.sunlight_intensity,
        dailyEvaporation: day.daily_evaporation,
        vapourPressureDeficit: day.vapour_pressure_deficit,
        relativeHumidity: day.relative_humidity_2m,
        windSpeedMph: day.wind_speed_10m,
      })),
    };

    this.logger.log(`Prediction payload built for plant ${plantInstanceId}`);
    this.logger.log(JSON.stringify(payload, null, 2));

    // 6. POST to FastAPI
    const mlUrl = process.env.ML_SERVICE_URL;
    if (!mlUrl) throw new BadRequestException('ML_SERVICE_URL not configured');

    const res = await fetch(`${mlUrl}/predict`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });

    if (!res.ok) {
      const err = await res.text();
      throw new BadRequestException(`ML service error: ${err}`);
    }

    return await res.json();
  }
}