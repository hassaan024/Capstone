import {
  Injectable,
  BadRequestException,
  NotFoundException,
} from '@nestjs/common';
import { CreateSpeciesDto } from './dto/create-species.dto';
import { UpdateSpeciesDto } from './dto/update-species.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';
import { TrefleService } from '../trefle/trefle.service.js';

@Injectable()
export class SpeciesService {
  constructor(
    private readonly db: DatabaseService,
    private readonly trefleService: TrefleService,
  ) {}

  async saveSpecies(userId: number, trefleId: number) {
    // 1. Check if species exists in our DB
    let species = await this.db.species.findUnique({
      where: { trefleId },
    });

    // 2. If not, fetch from Trefle and create it
    if (!species) {
      const trefleData = await this.trefleService.getDetails(trefleId);
      if (!trefleData || !trefleData.data) {
        throw new NotFoundException(`Species with trefleId ${trefleId} not found in Trefle`);
      }
      const plant = trefleData.data;

      // Map Trefle data to our schema
      // Note: We are using safe defaults or nulls for missing data
      // Use try-catch to handle race conditions where species might be created by another request
      try {
        species = await this.db.species.create({
          data: {
            commonName: plant.common_name || plant.scientific_name,
            scientificName: plant.scientific_name,
            trefleId: trefleId,
            imgSrcUrl: plant.image_url,
            growthRate: 0.0, // Default placeholders as Trefle might not have these specific numeric rates
            bloomRate: 0.0,
            witherRate: 0.0,
            wateringFreq: 'Average',
          },
        });
      } catch (err) {
        // If species was created by another request, fetch it
        if (err instanceof Prisma.PrismaClientKnownRequestError && err.code === 'P2002') {
          species = await this.db.species.findUnique({
            where: { trefleId },
          });
          if (!species) {
            throw new BadRequestException('Failed to create or find species');
          }
        } else {
          throw err;
        }
      }
    }

    // 3. Connect to user (use connect in a safe way that won't fail if already connected)
    await this.db.user.update({
      where: { id: userId },
      data: {
        savedSpecies: {
          connect: { id: species.id },
        },
      },
    });

    return { message: 'Species saved successfully', species };
  }

  async unsaveSpecies(userId: number, trefleId: number) {
    const species = await this.db.species.findUnique({
      where: { trefleId },
    });

    if (!species) {
      throw new NotFoundException('Species not found in database');
    }

    await this.db.user.update({
      where: { id: userId },
      data: {
        savedSpecies: {
          disconnect: { id: species.id },
        },
      },
    });

    return { message: 'Species unsaved successfully' };
  }

  async getSavedSpecies(userId: number) {
    const user = await this.db.user.findUnique({
      where: { id: userId },
      include: {
        savedSpecies: true,
      },
    });

    if (!user) throw new NotFoundException('User not found');
    return user.savedSpecies;
  }


  async create(createSpeciesDto: CreateSpeciesDto) {
    try {
      return await this.db.species.create({
        data: {
          ...createSpeciesDto,
        },
      });
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
    return this.db.species.findUnique({
      where: {
        id: id,
      },
    });
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
      return await this.db.species.delete({
        where: {
          id: id,
        },
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
}
