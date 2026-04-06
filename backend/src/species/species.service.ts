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

  // Get all species saved by a user
  async getSavedSpecies(userId: number) {
    const user = await this.db.user.findUnique({
      where: { id: userId },
      include: { savedSpecies: true },
    });

    if (!user) throw new NotFoundException('User not found');
    
    // Compute 'modelCategory' dynamically for Unreal Engine and React clients
    return user.savedSpecies.map((species) => {
      let modelCategory = 'flower'; // Default category
      const typeStr = (species.type || '').toLowerCase();

      if (
        typeStr.includes('vegetable') ||
        species.edibleFruit ||
        species.edibleLeaf
      ) {
        modelCategory = 'vegetable';
      } else if (typeStr.includes('tree') || typeStr.includes('shrub')) {
        modelCategory = 'tree';
      } else {
        modelCategory = 'flower';
      }

      return {
        ...species,
        modelCategory,
      };
    });
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
