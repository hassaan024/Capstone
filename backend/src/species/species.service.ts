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
import { computeBloomDays } from 'utils/plant-growth';

@Injectable()
export class SpeciesService {
  private readonly logger = new Logger(SpeciesService.name);

  constructor(
    private readonly db: DatabaseService,
    private readonly perenualService: PerenualService,
  ) {}

  // Save a species to a user's collection and persist bloom days
  async saveSpecies(userId: number, perenualId: number) {
    const species = await this.getOrCreateSpecies(perenualId);
    const bloomDays = computeBloomDays(species);

    const [updatedSpecies] = await Promise.all([
      this.db.species.update({
        where: { id: species.id },
        data: { bloomDays },
      }),
      this.db.user.update({
        where: { id: userId },
        data: { savedSpecies: { connect: { id: species.id } } },
      }),
    ]);

    return {
      message: 'Species saved successfully',
      species: { ...updatedSpecies, bloomDays },
    };
  }

  // Pin a species to a specific garden's saved list (a wishlist-style relation).
  // Does NOT create a PlantInstance — instances are only created when a user
  // actually plants and saves in Unreal.
  async saveSpeciesToGarden(
    userId: number,
    perenualId: number,
    gardenId: number,
  ) {
    const garden = await this.db.garden.findFirst({
      where: { id: gardenId, ownerId: userId },
    });
    if (!garden) throw new NotFoundException('Garden not found');

    const species = await this.getOrCreateSpecies(perenualId);
    const bloomDays = computeBloomDays(species);

    const [updatedSpecies] = await Promise.all([
      this.db.species.update({
        where: { id: species.id },
        data: { bloomDays },
      }),
      this.db.garden.update({
        where: { id: gardenId },
        data: { savedSpecies: { connect: { id: species.id } } },
      }),
    ]);

    return {
      message: 'Species saved to garden',
      species: { ...updatedSpecies, bloomDays },
    };
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

  // Unpin a species from a garden's saved list. Does NOT touch PlantInstances —
  // any real planted instances of this species in the garden remain untouched.
  async unsaveSpeciesFromGarden(
    userId: number,
    perenualId: number,
    gardenId: number,
  ) {
    const species = await this.db.species.findUnique({ where: { perenualId } });
    if (!species) throw new NotFoundException('Species not found in database');

    const garden = await this.db.garden.findFirst({
      where: { id: gardenId, ownerId: userId },
    });
    if (!garden) throw new NotFoundException('Garden not found');

    await this.db.garden.update({
      where: { id: gardenId },
      data: { savedSpecies: { disconnect: { id: species.id } } },
    });

    return { message: 'Species removed from garden' };
  }

  // Compute 'modelCategory' dynamically for Unreal Engine and React clients.
  // Rules are intentionally kept in sync with enrichWithModelCategory() in
  // perenual.service.ts so browse-plants and saved-plants always agree.
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

    // Flower: Perenual explicitly tagged it as a flower
    if (typeStr.includes('flower')) return 'flower';

    // Tree: Perenual explicitly tagged it as a tree (takes priority over edible signals)
    if (typeStr.includes('tree')) return 'tree';

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

    // Tree fallback: scientific-name hint
    if (species.scientificName?.includes('Malus')) return 'tree';

    return 'flower';
  }

  private mapSpeciesWithCategory(speciesList: any[]) {
    return speciesList.map((species) => ({
      ...species,
      commonName: species.commonName
        ? species.commonName.charAt(0).toUpperCase() +
          species.commonName.slice(1)
        : species.commonName,
      modelCategory: this.computeModelCategory(species),
      bloomDays: species.bloomDays ?? computeBloomDays(species),
    }));
  }

  // Get all species saved by a user, with bloom days and modelCategory
  async getSavedSpecies(userId: number) {
    const user = await this.db.user.findUnique({
      where: { id: userId },
      include: { savedSpecies: true },
    });

    if (!user) throw new NotFoundException('User not found');

    return this.mapSpeciesWithCategory(user.savedSpecies);
  }

  // Get all species pinned to a specific garden's saved list, with bloom days
  // and modelCategory. Reads the Garden ↔ Species relation directly — does NOT
  // derive from PlantInstance.
  async getSavedSpeciesForGarden(userId: number, gardenId: number) {
    const garden = await this.db.garden.findFirst({
      where: { id: gardenId, ownerId: userId },
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
