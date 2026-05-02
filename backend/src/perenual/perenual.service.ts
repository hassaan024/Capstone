import { Injectable, Logger } from '@nestjs/common';
import { DatabaseService } from 'database/database.service';
import { SearchPlantsQueryDto } from './dto/search-plants-query.dto';
import { celsiusToFahrenheit, cmToFeet, inchesToFeet, metersToFeet, mmToFeet } from 'utils/util-functions';
import { computeBloomDays } from 'utils/plant-growth';

const BASE_PERENUAL_URL = 'https://perenual.com/api/v2/';

// api interface for the deminsions
export interface Dimension {
  type: string | null;
  min_value: number | null;
  max_value: number | null;
  unit: string | null;
}

export interface MinMax {
  min: number | null;
  max: number | null;
}

@Injectable()
export class PerenualService {
  private readonly API_KEY = process.env.PERENUAL_API_TOKEN;
  private readonly logger = new Logger(PerenualService.name);

  constructor(private readonly db: DatabaseService) {}

  private enrichWithModelCategory(data: any) {
    if (!data) return data;

    // NOTE: The search-list endpoint does NOT return a `type` field, so plants
    // without explicit edible/vegetable signals fall through to 'flower' by default.
    // We keep the same rule for the detail endpoint for consistency with saved plants.
    const enrichPlant = (p: any) => {
      const typeStr = (p.type || p.cycle || '').toLowerCase();
      let modelCategory = 'flower';

      // If Perenual explicitly tagged it as a flower, trust that over edible flags
      if (typeStr.includes('flower')) {
        modelCategory = 'flower';
      } else if (
        // Vegetable: edible signals or explicit vegetable/herb/fruit type
        typeStr.includes('vegetable') ||
        p.edible_fruit ||
        p.edible_leaf ||
        p.cuisine ||
        typeStr.includes('herb') ||
        typeStr.includes('fruit') ||
        p.common_name?.toLowerCase().includes('tomato')
      ) {
        modelCategory = 'vegetable';
      } else if (p.scientific_name?.[0]?.includes('Malus')) {
        // Tree: only via scientific-name hints so behavior matches the list endpoint
        modelCategory = 'tree';
      }

      // Estimate bloom days via the growth model when Perenual provides the
      // required fields. If they're missing (e.g. the search-list endpoint),
      // omit it — the frontend will simply not show the pill.
      const growthRate = Number(p.growth_rate);
      const maxHeightFt = Number(
        p.dimensions?.max_value ?? p.dimension?.max_value,
      );
      const bloomDays =
        Number.isFinite(growthRate) && Number.isFinite(maxHeightFt)
          ? computeBloomDays({
              commonName: p.common_name ?? '',
              scientificName: Array.isArray(p.scientific_name)
                ? p.scientific_name[0] ?? ''
                : p.scientific_name ?? '',
              growthRate,
              maxHeight: maxHeightFt,
            })
          : undefined;

      return { ...p, modelCategory, bloomDays };
    };

    if (data.data && Array.isArray(data.data)) {
      data.data = data.data.map(enrichPlant);
    } else {
      return enrichPlant(data);
    }
    return data;
  }

  async searchPlants(queryDto: SearchPlantsQueryDto) {
    try {
      const url = `${BASE_PERENUAL_URL}species-list?key=${this.API_KEY}&q=${encodeURIComponent(
        queryDto.query,
      )}&page=${queryDto.page || 1}`;
      const response = await fetch(url, {
        signal: AbortSignal.timeout(30000),
      });
      if (!response.ok)
        throw new Error(`Failed to search plants: ${response.statusText}`);
      const data = await response.json();
      
      if (data?.data && Array.isArray(data.data)) {
        data.data = data.data.map((p: any) => {
          if (p.common_name) {
            p.common_name = p.common_name.charAt(0).toUpperCase() + p.common_name.slice(1);
          }
          return p;
        });
      }
      
      return data;
    } catch (err: any) {
      this.logger.error(err.message);
      throw err;
    }
  }


  async getPlantDetails(id: number) {
    try {
      this.logger.log(`Fetching plant details for ID ${id}`);
      const url = `${BASE_PERENUAL_URL}species/details/${id}?key=${this.API_KEY}`;

      const response = await fetch(url);
      if (!response.ok)
        throw new Error(`Failed to get plant details: ${response.statusText}`);

      const data = await response.json();
      
      if (data && data.common_name) {
        data.common_name = data.common_name.charAt(0).toUpperCase() + data.common_name.slice(1);
      }
      
      return this.enrichWithModelCategory(data);
    } catch (err: any) {
      this.logger.error(err.message);
      throw err;
    }
  }

  async importSpecies(perenualId: number) {
    const cachedSpecies = await this.db.species.findUnique({
      where: { perenualId },
    });

    // return cached entry, skip API request if it is already in DB
    if (cachedSpecies) {
      this.logger.log(`Species ${perenualId} found in cache`);
      return cachedSpecies; 
    }

    const data = await this.getPlantDetails(perenualId);

    const dimensions: Dimension[] = data.dimensions as Dimension[];

    // If you just want the first one (e.g., assuming "height") 
    const heightDimension: MinMax = dimensions.length ? this.parseDimensionsData(dimensions[0]) : { min: null, max: null };

    this.logger.log(heightDimension)

    const avgHoursSun = this.mapSunlightHours(data.sunlight || []);

    // proper watering parsing
    const { min: wateringMinDays, max: wateringMaxDays } =
      this.parseWateringBenchmark(data.watering_general_benchmark);

    //vtemperature normalization (logic kept)
    const { minTemp, maxTemp } = this.normalizeTemperature(
      data.hardiness?.min ? Number(data.hardiness.min) : null,
      data.hardiness?.max ? Number(data.hardiness.max) : null,
      data.sunlight || [],
      data.type,
    );

    const speciesData = {
      commonName: data.common_name || 'Unknown',
      scientificName: data.scientific_name?.[0] || 'Unknown',
      otherNames: data.other_name || [],
      family: data.family || null,
      genus: data.genus || null,
      speciesEpithet: data.species_epithet || null,
      origin: data.origin || [],
      type: data.type || null,
      cycle: data.cycle || null,
      growthRate: this.mapGrowthRate(data.growth_rate) ?? 2,

      wateringFreq: data.watering || null,
      wateringMinDays,
      wateringMaxDays,

      minTemp,
      maxTemp,
      avgHoursSun,

      minHeight: heightDimension.min,
      maxHeight: heightDimension.max,

      pruningMonths: data.pruning_month || [],
      pruningFrequency: data.pruning_count?.amount || null,
      pruningInterval: data.pruning_count?.interval || null,

      maintenance: data.maintenance || null,
      careLevel: data.care_level || null,

      droughtTolerant: Boolean(data.drought_tolerant),
      saltTolerant: Boolean(data.salt_tolerant),
      thorny: Boolean(data.thorny),
      invasive: Boolean(data.invasive),
      tropical: Boolean(data.tropical),
      indoor: Boolean(data.indoor),

      flowers: Boolean(data.flowers),
      fruits: Boolean(data.fruits),
      edibleFruit: Boolean(data.edible_fruit),
      leaf: Boolean(data.leaf),
      edibleLeaf: Boolean(data.edible_leaf),
      cuisine: Boolean(data.cuisine),
      medicinal: Boolean(data.medicinal),

      floweringSeason: data.flowering_season || null,
      harvestSeason: data.harvest_season || null,
      notes: data.description || null,

      plantAnatomy: data.plant_anatomy || [],

      imgSrcUrls: {
        original: data.default_image?.original_url || null,
        regular: data.default_image?.regular_url || null,
        medium: data.default_image?.medium_url || null,
        small: data.default_image?.small_url || null,
        thumbnail: data.default_image?.thumbnail || null,
      },

      perenualId: data.id,
      trefleId: null,
    };

    return this.db.species.upsert({
      where: { perenualId: speciesData.perenualId },
      create: speciesData,
      update: speciesData,
    });
  }

  // ---------------- HELPERS ----------------

  private mapGrowthRate(rate: string | null): number {
    if (!rate) return 2;
    const map = { Low: 1, Medium: 2, High: 3 };
    return map[rate] ?? 2;
  }

  private mapSunlightHours(sunlight: string[]): number | null {
    if (!sunlight || sunlight.length === 0) return null;

    let total = 0;
    // approximat the numeric values
    sunlight.forEach((s) => {
      const val = s.toLowerCase();
      if (val.includes('full sun')) total += 10;
      else if (val.includes('part shade')) total += 6;
      else if (val.includes('shade')) total += 3;
    });

    return total / sunlight.length;
  }

  // deminsions parser
  private parseDimensionsData(
    dimensions: Dimension | null
  ): MinMax {
    const res: MinMax = { min: null, max: null };

    if (!dimensions) return res;

    let min: number | null = dimensions.min_value ?? null;
    let max: number | null = dimensions.max_value ?? null;

    // if min or max is missing, return early
    if (min === null || max === null) return res;

    // normalize units
    const units: string = (dimensions.unit || 'feet').toLowerCase();

    switch (units) {
      case 'inches':
      case 'in':
        min = inchesToFeet(min);
        max = inchesToFeet(max);
        break;
      case 'meters':
      case 'm':
        min = metersToFeet(min);
        max = metersToFeet(max);
        break;
      case 'centimeters':
      case 'cm':
        min = cmToFeet(min);
        max = cmToFeet(max);
        break;
      case 'millimeters':
      case 'mm':
        min = mmToFeet(min);
        max = mmToFeet(max);
        break;
      case 'feet':
      default:
        // do nothing, already in feet
        break;
    }

    res.min = min;
    res.max = max;
    return res;
  }

  // watering parser
  private parseWateringBenchmark(
    benchmark: { value: string; unit: string } | null,
  ): MinMax {
    if (!benchmark?.value) return { min: null, max: null };

    const cleaned = benchmark.value.replace(/"/g, '').trim();

    if (cleaned.includes('-')) {
      const [minStr, maxStr] = cleaned.split('-');
      const min = parseInt(minStr, 10);
      const max = parseInt(maxStr, 10);

      return {
        min: isNaN(min) ? null : min,
        max: isNaN(max) ? null : max,
      };
    }

    const value = parseInt(cleaned, 10);
    return {
      min: isNaN(value) ? null : value,
      max: isNaN(value) ? null : value,
    };
  }

  private normalizeTemperature(
    zoneMin: number | null,
    zoneMax: number | null,
    sunlight: string[] = [],
    type?: string,
  ): { minTemp: number | null; maxTemp: number | null } {
    if (!zoneMin && !zoneMax) {
      return { minTemp: null, maxTemp: null };
    }

    const zoneMap: Record<number, number> = {
      1: -51, 2: -45, 3: -40, 4: -34, 5: -29,
      6: -23, 7: -18, 8: -12, 9: -7, 10: -1,
      11: 4, 12: 10, 13: 16,
    };

    const minSurvival = zoneMin ? zoneMap[zoneMin] : null;
    const maxSurvival = zoneMax ? zoneMap[zoneMax] + 10 : null;

    let minTemp = minSurvival !== null ? minSurvival + 10 : null;
    let maxTemp = maxSurvival !== null ? maxSurvival + 15 : null;

    const hasFullSun = sunlight.some((s) =>
      s.toLowerCase().includes('full sun'),
    );
    const hasShade = sunlight.some((s) =>
      s.toLowerCase().includes('shade'),
    );

    if (hasFullSun && maxTemp !== null) maxTemp += 5;
    if (hasShade && maxTemp !== null) maxTemp -= 5;

    if (type?.toLowerCase() === 'fruit') {
      if (minTemp !== null) minTemp = Math.max(minTemp, 10);
      if (maxTemp !== null) maxTemp = Math.max(maxTemp, 25);
    }

    if (minTemp !== null) minTemp = Math.max(minTemp, -10);
    if (maxTemp !== null) maxTemp = Math.min(maxTemp, 45);

    return {
      minTemp: minTemp !== null ? celsiusToFahrenheit(Math.round(minTemp)) : null,
      maxTemp: maxTemp !== null ? celsiusToFahrenheit(Math.round(maxTemp)) : null,
    };
  }
}