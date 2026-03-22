import {
  Injectable,
  BadRequestException,
  NotFoundException,
  Logger,
} from '@nestjs/common';
import { CreateSpeciesDto } from './dto/create-species.dto';
import { UpdateSpeciesDto } from './dto/update-species.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';
import { PerenualService } from 'perenual/perenual.service';

@Injectable()
export class SpeciesService {
  private readonly logger = new Logger(SpeciesService.name);

  constructor(
    private readonly db: DatabaseService,
    private readonly perenualService: PerenualService,
  ) {}

  // Save a species to a user's collection
  async saveSpecies(userId: number, perenualId: number) {
    const species = await this.getOrCreateSpecies(perenualId);

    await this.db.user.update({
      where: { id: userId },
      data: { savedSpecies: { connect: { id: species.id } } },
    });

    return { message: 'Species saved successfully', species };
  }

  // Fetch from DB or Perenual API, then create or upsert in DB
  private async getOrCreateSpecies(perenualId: number) {
    // Check DB first
    let species = await this.db.species.findUnique({ where: { perenualId } });
    if (species) return species;

    // Fetch from Perenual API
    const data = await this.perenualService.getPlantDetails(perenualId);
    if (!data) throw new NotFoundException(`Species ${perenualId} not found`);

    const avgHoursSun = this.mapSunlightHours(data.sunlight || []);

    const speciesData = {
      commonName: data.common_name || 'Unknown',
      scientificName: data.scientific_name?.[0] || null,
      otherNames: data.other_name || [],
      family: data.family || null,
      genus: data.genus || null,
      speciesEpithet: data.species_epithet || null,
      origin: data.origin || [],
      type: data.type || null,
      cycle: data.cycle || null,
      growthRate: this.mapGrowthRate(data.growth_rate),
      wateringFreq: data.watering || null,
      wateringBenchmark: data.watering_general_benchmark || null,
      minTemp: data.hardiness?.min ? Number(data.hardiness.min) : null,
      maxTemp: data.hardiness?.max ? Number(data.hardiness.max) : null,
      avgHoursSun,
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

      // External IDs
      perenualId: data.id,
      trefleId: null, // optional legacy field
    };

    // Upsert to avoid duplicates
    return this.db.species.upsert({
      where: { perenualId },
      create: speciesData,
      update: speciesData,
    });
  }

  // Remove a species from a user's saved collection
  async unsaveSpecies(userId: number, perenualId: number) {
    const species = await this.db.species.findUnique({ where: { perenualId } });
    if (!species) throw new NotFoundException('Species not found in database');

    await this.db.user.update({
      where: { id: userId },
      data: { savedSpecies: { disconnect: { id: species.id } } },
    });

    return { message: 'Species unsaved successfully' };
  }

  // Get all species saved by a user
  async getSavedSpecies(userId: number) {
    const user = await this.db.user.findUnique({
      where: { id: userId },
      include: { savedSpecies: true },
    });

    if (!user) throw new NotFoundException('User not found');
    return user.savedSpecies;
  }

  // Standard CRUD operations
  async create(createSpeciesDto: CreateSpeciesDto) {
    try {
      return await this.db.species.create({ data: createSpeciesDto });
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2002'
      ) {
        const target = err.meta?.target as string[] | undefined;
        if (target?.includes('commonName')) {
          throw new BadRequestException(
            'A species with this commonName already exists.',
          );
        }
        if (target?.includes('scientificName')) {
          throw new BadRequestException(
            'A species with this scientificName already exists.',
          );
        }
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  findAll() {
    return this.db.species.findMany();
  }

  findOne(id: number) {
    return this.db.species.findUnique({ where: { id } });
  }

  async update(id: number, updateSpeciesDto: UpdateSpeciesDto) {
    try {
      return await this.db.species.update({
        where: { id },
        data: updateSpeciesDto,
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Species with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      return await this.db.species.delete({ where: { id } });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Species with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  // Helper: map growth rate string to numeric value
  private mapGrowthRate(rate: string | null): number | null {
    if (!rate) return null;
    const map = { Low: 1, Medium: 2, High: 3 };
    return map[rate] ?? null;
  }

  // Helper: convert sunlight descriptions to average hours
  private mapSunlightHours(sunlight: string[]): number | null {
    if (!sunlight || sunlight.length === 0) return null;
    let total = 0;
    sunlight.forEach((s) => {
      if (s.includes('full sun')) total += 10;
      else if (s.includes('part shade')) total += 6;
      else if (s.includes('full shade')) total += 3;
    });
    return total / sunlight.length;
  }
}

  // constructor(
  //   private readonly db: DatabaseService,
  //   private readonly trefleService: TrefleService,
  // ) {}

  // async saveSpecies(userId: number, trefleId: number) {
  //   const species = await this.getOrCreateSpecies(trefleId);

  //   // Attach to user
  //   await this.db.user.update({
  //     where: { id: userId },
  //     data: {
  //       savedSpecies: { connect: { id: species.id } },
  //     },
  //   });

  //   return { message: 'Species saved successfully', species };
  // }

  // /** Private helper: fetch from DB or Trefle */
  // private async getOrCreateSpecies(trefleId: number) {
  //   //  Check if it exists in our DB
  //   // let species = await this.db.species.findUnique({ where: { trefleId } });
  //   // if (species) return species;

  //   // Fetch from Trefle
  //   const trefleData = await this.trefleService.getDetails(trefleId);
  //   const plant = trefleData?.data;

  //   this.logger.log(JSON.stringify(plant, null, 2));

  //   if (!plant) throw new NotFoundException(`Species ${trefleId} not found in Trefle`);

  //   // create in DB 
  //   try {
  //     return await this.db.species.create({
  //       data: {
  //         commonName: plant.common_name || plant.scientific_name,
  //         scientificName: plant.scientific_name,
  //         trefleId,
  //         imgSrcUrl: plant.image_url ?? null,
  //         growthRate: 0,
  //         bloomRate: 0,
  //         witherRate: 0,
  //         wateringFreq: 'Average',
  //       },
  //     });
  //   } catch (err) {
  //     if (err instanceof Prisma.PrismaClientKnownRequestError && err.code === 'P2002') {
  //       // Already created by another request, fetch it again
  //       const retry = await this.db.species.findUnique({ where: { trefleId } });
  //       if (!retry) throw new BadRequestException('Failed to create or find species');
  //       return retry;
  //     }
  //     throw err;
  //   }
  // }

  // async unsaveSpecies(userId: number, trefleId: number) {
  //   const species = await this.db.species.findUnique({
  //     where: { trefleId },
  //   });

  //   if (!species) {
  //     throw new NotFoundException('Species not found in database');
  //   }

  //   await this.db.user.update({
  //     where: { id: userId },
  //     data: {
  //       savedSpecies: {
  //         disconnect: { id: species.id },
  //       },
  //     },
  //   });

  //   return { message: 'Species unsaved successfully' };
  // }

