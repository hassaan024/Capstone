import {
  Injectable,
  Logger,
  NotFoundException,
  BadRequestException,
} from '@nestjs/common';
import { DatabaseService } from 'database/database.service';
import { WeatherService } from 'weather/weather.service';
import { WeatherInfoDto } from 'weather/dto/weather-info.dto';
import { estimateSoilMoisture } from 'utils/soil-moisture';
import {
  estimateDaysToMature,
  rawDaysToMature,
  growthStageFromRatio,
  TIMELINE_INTERVAL_DAYS,
} from 'utils/plant-growth';
import { computeModelCategory } from 'utils/model-category';
import { GrowthStage } from 'enums/table_enums';
import { DEMO_SPECIES, getDemoSpecies } from 'utils/demo-species';

interface TimelineEntry {
  historyId: number;
  date: string;
  dayIndex: number;
  heightCm: number;
  growthStage: GrowthStage;
  stressFactors: {
    temperature: number;
    sunlight: number;
    water: number;
    soil: number;
    overall: number;
  };
  confidence: number;
}

// Shape that buildPayload expects for species — a flat set of ML-relevant fields
interface SpeciesPayloadData {
  growthRate: number;
  minHeight: number | null;
  maxHeight: number | null;
  avgHoursSun: number | null;
  minTemp: number | null;
  maxTemp: number | null;
  wateringMinDays: number | null;
  wateringMaxDays: number | null;
  droughtTolerant: boolean | null;
  tropical: boolean | null;
  cycle: string | null;
  modelCategory: string | null;
}

@Injectable()
export class PredictionService {
  private readonly logger = new Logger(PredictionService.name);

  constructor(
    private readonly db: DatabaseService,
    private readonly weatherService: WeatherService,
  ) {}

  async predict(plantInstanceId: number, daysAhead: number) {
    const plant = await this.db.plantInstance.findUnique({
      where: { id: plantInstanceId },
      include: { species: true, soil: true, garden: true },
    });

    if (!plant) throw new NotFoundException(`Plant instance ${plantInstanceId} not found`);

    const { soil, garden } = plant;

    // Use accurate hardcoded data for demo species if available
    const speciesData = this.resolveSpecies(plant.species);

    const ageDays = Math.floor(
      (plant.currentGameDate.getTime() - plant.plantedDate.getTime()) / 86400000,
    );

    const weather = await this.weatherService.getWeatherForGameDate(
      garden.latitude,
      garden.longitude,
      plant.currentGameDate,
      Math.min(daysAhead, 30),
    );

    const soilMoisture = estimateSoilMoisture(weather, soil.type);
    const payload = this.buildPayload(plant, speciesData, soil, weather, soilMoisture, ageDays, daysAhead);

    this.logger.log(`Prediction payload built for plant ${plantInstanceId}`);

    const result = await this.callMlService(payload);
    return {
      plant: {
        plantInstanceId,
        commonName: plant.species.commonName,
        scientificName: plant.species.scientificName,
        type: plant.species.type,
      },
      ...result,
    };
  }

  async generateTimeline(gardenId: number, bloomDate: Date) {
    const garden = await this.db.garden.findUnique({
      where: { id: gardenId },
      include: {
        plants: { include: { species: true, soil: true } },
      },
    });

    if (!garden) throw new NotFoundException(`Garden ${gardenId} not found`);
    if (!garden.plants.length) throw new BadRequestException(`Garden ${gardenId} has no plants`);

    // Pre-compute per-plant metadata synchronously so we can determine the
    // full weather window needed before making any API calls.
    const plantMeta = garden.plants.map((plant) => {
      const speciesData = this.resolveSpecies(plant.species);
      const maxHeightCm = (speciesData.maxHeight ?? 100) * 30.48;
      // const demoData = getDemoSpecies(plant.species.commonName, plant.species.scientificName);
      const demoData = null;
      const daysNeeded = rawDaysToMature(speciesData.growthRate, maxHeightCm, speciesData.modelCategory, speciesData.cycle);
      const daysToMature = Math.min(daysNeeded, 730);
      const feasible = daysNeeded <= 730;
      const plantedDate = new Date(bloomDate.getTime() - daysToMature * 86400000);
      return { plant, speciesData, maxHeightCm, demoData, daysNeeded, daysToMature, feasible, plantedDate };
    });

    // Fetch base window + 365-day buffer so simulations can run longer than
    // daysToMature when conditions are poor. Each plant's offset positions it
    // within the shared array; the buffer sits after every plant's base end.
    const maxDaysToMature = Math.max(...plantMeta.map((m) => m.daysToMature));
    const bufferedDays = Math.min(maxDaysToMature + 365, 730);
    const earliestPlantedDate = new Date(bloomDate.getTime() - bufferedDays * 86400000);

    const sharedWeather = await this.weatherService.getWeatherForGameDate(
      garden.latitude,
      garden.longitude,
      earliestPlantedDate,
      bufferedDays + TIMELINE_INTERVAL_DAYS,
    );

    const plantResults = await Promise.all(
      plantMeta.map(async ({ plant, speciesData, maxHeightCm, demoData, daysNeeded, daysToMature, feasible, plantedDate }) => {
        // plantOffset positions this plant's seasonal window within the shared array.
        // The 365-day buffer sits after index maxDaysToMature, giving each plant
        // room to run longer than daysToMature when conditions are poor.
        const plantOffset = maxDaysToMature - daysToMature;

        await this.db.plantHistory.deleteMany({ where: { plantId: plant.id } });

        let currentHeight = 0;
        let dayIndex = 0;
        const timeline: TimelineEntry[] = [];

        // Run until the plant reaches 90% of max height or weather data runs out.
        // This makes the timeline duration weather-driven: bad conditions = longer timeline.
        while (true) {
          const weatherSlice = sharedWeather.slice(plantOffset + dayIndex, plantOffset + dayIndex + TIMELINE_INTERVAL_DAYS);
          if (!weatherSlice.length) break;

          const gameDate = new Date(plantedDate.getTime() + dayIndex * 86400000);
          const heightRatio = maxHeightCm > 0 ? currentHeight / maxHeightCm : 0;
          const growthStage = growthStageFromRatio(heightRatio);

          const soilMoisture = estimateSoilMoisture(weatherSlice, plant.soil.type);

          const payload = this.buildPayload(
            { ...plant, heightCm: currentHeight, growthStage, bloomState: heightRatio >= 0.9 },
            speciesData,
            plant.soil,
            weatherSlice,
            soilMoisture,
            dayIndex,
            TIMELINE_INTERVAL_DAYS,
          );

          const prediction = await this.callMlService(payload);
          currentHeight = Math.min(prediction.predictedHeightCm as number, maxHeightCm);

          const record = await this.db.plantHistory.create({
            data: {
              plantId: plant.id,
              date: gameDate,
              heightCm: currentHeight,
              growthStage,
              predictedValue: prediction.stressFactors.overall as number,
            },
          });

          timeline.push({
            historyId: record.id,
            date: gameDate.toISOString().split('T')[0],
            dayIndex,
            heightCm: currentHeight,
            growthStage,
            stressFactors: prediction.stressFactors as TimelineEntry['stressFactors'],
            confidence: prediction.confidence as number,
          });

          if (currentHeight >= maxHeightCm * 0.9) break;
          dayIndex += TIMELINE_INTERVAL_DAYS;
        }

        // plantedDate is now weather-driven: how far back from bloomDate the
        // simulation actually needed to run to reach mature height.
        const actualDaysToMature = dayIndex + TIMELINE_INTERVAL_DAYS;
        const actualPlantedDate = new Date(bloomDate.getTime() - actualDaysToMature * 86400000);

        await this.db.plantInstance.update({
          where: { id: plant.id },
          data: { plantedDate: actualPlantedDate, currentGameDate: bloomDate },
        });

        const earliestFeasibleBloomDate = !feasible
          ? new Date(plantedDate.getTime() + daysNeeded * 86400000).toISOString().split('T')[0]
          : null;

        // Derive suitability from the already-computed stress factors.
        // No extra API calls — we aggregate what the ML model already returned.
        const suitabilityReasons: string[] = [];
        if (timeline.length > 0) {
          const avg = (fn: (e: TimelineEntry) => number) =>
            timeline.reduce((sum, e) => sum + fn(e), 0) / timeline.length;

          const avgTempStress    = avg(e => e.stressFactors.temperature);
          const avgSunStress     = avg(e => e.stressFactors.sunlight);
          const avgWaterStress   = avg(e => e.stressFactors.water);
          const avgOverallStress = avg(e => e.stressFactors.overall);

          if (avgTempStress < 0.7)
            suitabilityReasons.push(
              `Temperature is outside the comfortable range for ${plant.species.commonName} throughout most of the growth period.`,
            );
          if (avgSunStress < 0.7)
            suitabilityReasons.push(
              `Available sunlight is consistently below what ${plant.species.commonName} needs to grow well.`,
            );
          if (avgWaterStress < 0.7)
            suitabilityReasons.push(
              `Soil moisture and precipitation are too low for ${plant.species.commonName} during this period.`,
            );
          if (avgOverallStress < 0.5)
            suitabilityReasons.push(
              `Combined growing conditions are too poor for meaningful growth — consider a different planting window.`,
            );
        }
        const suitable = suitabilityReasons.length === 0;

        return {
          plantInstanceId: plant.id,
          speciesName: plant.species.commonName,
          isDemo: !!demoData,
          feasible,
          feasibilityNote: !feasible
            ? `${plant.species.commonName} needs ~${daysNeeded} days to reach full height but the timeline is capped at 730. ` +
              `The slider will show partial growth (~${Math.round((730 / daysNeeded) * 100)}% of max height). ` +
              `Earliest feasible bloom date: ${earliestFeasibleBloomDate}.`
            : null,
          suitable,
          suitabilityReasons,
          plantedDate: actualPlantedDate.toISOString().split('T')[0],
          daysToMature: actualDaysToMature,
          maxHeightCm,
          timeline,
        };
      }),
    );

    return {
      gardenId,
      bloomDate: bloomDate.toISOString().split('T')[0],
      plants: plantResults,
    };
  }

  /**
   * Returns an estimated bloom date for a species given an optional planted date.
   * No ML call — pure formula. Used by the frontend to show "~X months to full bloom."
   */
  async bloomEstimate(speciesId: number, plantedDate: Date, gardenId?: number) {
    const species = await this.db.species.findUnique({ where: { id: speciesId } });
    if (!species) throw new NotFoundException(`Species ${speciesId} not found`);

    const speciesData = this.resolveSpecies(species);
    const maxHeightCm = (speciesData.maxHeight ?? 100) * 30.48;
    // const demoData = getDemoSpecies(species.commonName, species.scientificName);
    const daysNeeded = rawDaysToMature(speciesData.growthRate, maxHeightCm, speciesData.modelCategory, speciesData.cycle);
    const daysToMature = Math.min(daysNeeded, 730);
    const feasible = daysNeeded <= 730;

    const estimatedBloomDate = new Date(plantedDate.getTime() + daysToMature * 86400000)
      .toISOString().split('T')[0];

    // If gardenId provided, fetch current weather and check if conditions suit this species
    let suitability: { suitable: boolean; reasons: string[] } | null = null;
    if (gardenId) {
      const garden = await this.db.garden.findUnique({ where: { id: gardenId } });
      if (!garden) throw new NotFoundException(`Garden ${gardenId} not found`);
      const weather = await this.weatherService.getCurrentDaysWeather(garden.latitude, garden.longitude);
      suitability = this.checkSuitability(speciesData, weather);
    }

    return {
      speciesId,
      speciesName: species.commonName,
      plantedDate: plantedDate.toISOString().split('T')[0],
      estimatedBloomDate,
      daysToMature,
      maxHeightCm: Math.round(maxHeightCm),
      feasible,
      feasibilityNote: !feasible
        ? `${species.commonName} needs ~${daysNeeded} days to reach full height. ` +
          `Showing estimate based on 730 days (~${Math.round((730 / daysNeeded) * 100)}% of max height).`
        : null,
      suitability,
    };
  }

  private checkSuitability(
    species: {
      minTemp: number | null;
      maxTemp: number | null;
      avgHoursSun: number | null;
      droughtTolerant: boolean | null;
    },
    weather: WeatherInfoDto,
  ): { suitable: boolean; reasons: string[] } {
    const reasons: string[] = [];

    // Temperature check (weather already in °F)
    if (species.minTemp !== null && weather.temperature_2m !== undefined) {
      if (weather.temperature_2m < species.minTemp) {
        reasons.push(
          `Current temp ${Math.round(weather.temperature_2m)}°F is below the minimum ${species.minTemp}°F this species needs`,
        );
      }
    }
    if (species.maxTemp !== null && weather.temperature_2m !== undefined) {
      if (weather.temperature_2m > species.maxTemp) {
        reasons.push(
          `Current temp ${Math.round(weather.temperature_2m)}°F exceeds the maximum ${species.maxTemp}°F this species tolerates`,
        );
      }
    }

    // Sunlight check — shortwave_radiation_sum is MJ/m², ~3.6 MJ per peak sun hour
    if (species.avgHoursSun !== null && weather.sunlight_intensity !== undefined) {
      const estimatedHours = weather.sunlight_intensity / 3.6;
      if (estimatedHours < species.avgHoursSun * 0.7) {
        reasons.push(
          `Estimated sunlight today (~${estimatedHours.toFixed(1)} hrs) is below the ${species.avgHoursSun} hrs/day this species needs`,
        );
      }
    }

    // Drought / moisture check
    if (!species.droughtTolerant && weather.precipitation !== undefined && weather.precipitation < 1) {
      reasons.push(
        `Low precipitation today (${weather.precipitation}mm) — this species is not drought tolerant and needs consistent moisture`,
      );
    }

    return { suitable: reasons.length === 0, reasons };
  }

  /**
   * Upserts the three Louisiana demo species into the database.
   * Safe to call multiple times — always brings the records up to date with
   * the hardcoded values in demo-species.ts.
   * Returns the DB record for each species including its assigned ID.
   */
  async seedDemoSpecies() {
    const results: { action: string; id: number; commonName: string; type: string }[] = [];

    for (const demo of DEMO_SPECIES) {
      const existing = await this.db.species.findFirst({
        where: { scientificName: demo.scientificName },
      });

      const data = {
        commonName: demo.commonName,
        scientificName: demo.scientificName,
        growthRate: demo.growthRate,
        minHeight: demo.minHeight,
        maxHeight: demo.maxHeight,
        avgHoursSun: demo.avgHoursSun,
        minTemp: demo.minTemp,
        maxTemp: demo.maxTemp,
        wateringMinDays: demo.wateringMinDays,
        wateringMaxDays: demo.wateringMaxDays,
        droughtTolerant: demo.droughtTolerant,
        tropical: demo.tropical,
        cycle: demo.cycle,
        type: demo.type,
        modelCategory: computeModelCategory(demo),
        careLevel: demo.careLevel,
        family: demo.family,
        genus: demo.genus,
        speciesEpithet: demo.speciesEpithet,
        origin: demo.origin,
        flowers: demo.flowers,
        floweringSeason: demo.floweringSeason,
        fruits: demo.fruits,
        edibleFruit: demo.edibleFruit,
        notes: `${demo.notes}\n\nSowing info (North Louisiana, Zone 8a): ${demo.sowingInfo}`,
      };

      if (existing) {
        await this.db.species.update({ where: { id: existing.id }, data });
        results.push({ action: 'updated', id: existing.id, commonName: demo.commonName, type: demo.type });
        this.logger.log(`Demo species updated: ${demo.commonName} (id=${existing.id})`);
      } else {
        const created = await this.db.species.create({ data });
        results.push({ action: 'created', id: created.id, commonName: demo.commonName, type: demo.type });
        this.logger.log(`Demo species created: ${demo.commonName} (id=${created.id})`);
      }
    }

    return {
      message: 'Demo species seeded successfully. Use these speciesIds when creating plant instances.',
      species: results,
    };
  }

  // ── Helpers ──────────────────────────────────────────────────────────────

  /**
   * Returns accurate hardcoded species data if this is a demo species,
   * otherwise returns the DB species record unchanged.
   * Always ensures modelCategory is populated.
   */
  private resolveSpecies(dbSpecies: {
    commonName: string;
    scientificName: string;
    growthRate: number;
    minHeight: number | null;
    maxHeight: number | null;
    avgHoursSun: number | null;
    minTemp: number | null;
    maxTemp: number | null;
    wateringMinDays: number | null;
    wateringMaxDays: number | null;
    droughtTolerant: boolean | null;
    tropical: boolean | null;
    cycle: string | null;
    modelCategory?: string | null;
    type?: string | null;
    edibleFruit?: boolean | null;
    edibleLeaf?: boolean | null;
    cuisine?: boolean | null;
  }): SpeciesPayloadData {
    // const demo = getDemoSpecies(dbSpecies.commonName, dbSpecies.scientificName);
    // if (demo) {
    //   this.logger.log(`Using hardcoded demo data for species: ${dbSpecies.commonName}`);
    //   return {
    //     ...demo,
    //     modelCategory: computeModelCategory(demo),
    //   };
    // }
    return {
      ...dbSpecies,
      modelCategory: dbSpecies.modelCategory ?? computeModelCategory(dbSpecies),
    };
  }

  private buildPayload(
    plant: {
      id: number;
      heightCm: number | null;
      growthStage: string | null; // accepts both local GrowthStage enum and Prisma's $Enums.GrowthStage
      healthStatus: string | null;
      bloomState: boolean;
    },
    species: SpeciesPayloadData,
    soil: {
      type: string;
      pH: number;
      nitrogen: number;
      phosphorus: number;
      potassium: number;
      organicPercentage: number | null;
    },
    weather: WeatherInfoDto[],
    soilMoisture: number,
    ageDays: number,
    daysAhead: number,
  ) {
    return {
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
        growthRate: species.growthRate ?? 2,
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
        modelCategory: species.modelCategory ?? 'flower',
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
  }

  private async callMlService(payload: object) {
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

    return res.json();
  }
}
