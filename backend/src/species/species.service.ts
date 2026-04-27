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
    return this.perenualService.importSpecies(perenualId);
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

  // Compute 'modelCategory' dynamically for Unreal Engine and React clients.
  // Rules are intentionally kept in sync with enrichWithModelCategory() in
  // perenual.service.ts so browse-plants and saved-plants always agree.
  //
  // NOTE: The browse search-list API does NOT return a `type` field, so plants
  // without explicit edible/vegetable signals naturally fall through to 'flower'.
  // We replicate that behavior here by NOT using species.type for tree detection
  // — only scientific-name hints identify trees, everything else defaults to flower.
  private computeModelCategory(species: {
    type?: string | null;
    cycle?: string | null;
    edibleFruit?: boolean | null;
    edibleLeaf?: boolean | null;
    cuisine?: boolean | null;
    commonName?: string | null;
    scientificName?: string | null;
  }) {
    const typeStr = (species.type || species.cycle || '').toLowerCase();

    // If Perenual explicitly tagged it as a flower, trust that over edible flags
    // (e.g. sunflowers are edible but are flower models, not vegetable models)
    if (typeStr.includes('flower')) return 'flower';

    // Vegetable: edible signals or explicit vegetable/herb/fruit type
    if (
      typeStr.includes('vegetable') ||
      species.edibleFruit ||
      species.edibleLeaf ||
      species.cuisine ||
      typeStr.includes('herb') ||
      typeStr.includes('fruit') ||
      species.commonName?.toLowerCase().includes('tomato')
    ) {
      return 'vegetable';
    }

    // Tree: only via scientific-name hints (mirrors browse list which has no type field)
    if (species.scientificName?.includes('Malus')) {
      return 'tree';
    }

    return 'flower';
  }

  private mapSpeciesWithCategory(speciesList: any[]) {
    return speciesList.map((species) => {
      const modelCategory = this.computeModelCategory(species);
      let daysToBloom = 0;
      if (modelCategory === 'flower') daysToBloom = 20;
      else if (modelCategory === 'vegetable') daysToBloom = 30;
      else if (modelCategory === 'tree') daysToBloom = 50;

      return {
        ...species,
        commonName: species.commonName ? species.commonName.charAt(0).toUpperCase() + species.commonName.slice(1) : species.commonName,
        modelCategory,
        daysToBloom,
      };
    });
  }

  // Get all species saved by a user (globally)
  async getSavedSpecies(userId: number) {
    const user = await this.db.user.findUnique({
      where: { id: userId },
      include: { savedSpecies: true },
    });

    if (!user) throw new NotFoundException('User not found');
    return this.mapSpeciesWithCategory(user.savedSpecies);
  }

  // ─── Garden-scoped save operations ──────────────────────────────

  /** Verify the garden belongs to the given user */
  private async verifyGardenOwnership(gardenId: number, userId: number) {
    const garden = await this.db.garden.findFirst({
      where: { id: gardenId, ownerId: userId },
    });
    if (!garden) {
      throw new NotFoundException(
        `Garden ${gardenId} not found or not owned by user ${userId}`,
      );
    }
    return garden;
  }

  /** Save a species to a specific garden */
  async saveSpeciesToGarden(
    userId: number,
    perenualId: number,
    gardenId: number,
  ) {
    await this.verifyGardenOwnership(gardenId, userId);
    const species = await this.getOrCreateSpecies(perenualId);

    await this.db.garden.update({
      where: { id: gardenId },
      data: { savedSpecies: { connect: { id: species.id } } },
    });

    return { message: 'Species saved to garden successfully', species };
  }

  /** Remove a species from a garden's saved list */
  async unsaveSpeciesFromGarden(
    userId: number,
    perenualId: number,
    gardenId: number,
  ) {
    await this.verifyGardenOwnership(gardenId, userId);
    const species = await this.db.species.findUnique({ where: { perenualId } });
    if (!species) throw new NotFoundException('Species not found in database');

    await this.db.garden.update({
      where: { id: gardenId },
      data: { savedSpecies: { disconnect: { id: species.id } } },
    });

    return { message: 'Species unsaved from garden successfully' };
  }

  /** Get all species saved to a specific garden */
  async getSavedSpeciesForGarden(userId: number, gardenId: number) {
    await this.verifyGardenOwnership(gardenId, userId);

    const garden = await this.db.garden.findUnique({
      where: { id: gardenId },
      include: { savedSpecies: true },
    });

    if (!garden) throw new NotFoundException('Garden not found');
    return this.mapSpeciesWithCategory(garden.savedSpecies);
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
}
