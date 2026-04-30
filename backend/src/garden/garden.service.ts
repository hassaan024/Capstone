import {
  Injectable,
  BadRequestException,
  NotFoundException,
} from '@nestjs/common';
import { CreateGardenDto } from './dto/create-garden.dto';
import { UpdateGardenDto } from './dto/update-garden.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class GardenService {
  constructor(private readonly db: DatabaseService) {}

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

    if (typeStr.includes('flower')) return 'flower';

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

    if (species.scientificName?.includes('Malus')) {
      return 'tree';
    }

    return 'flower';
  }

  private mapSpeciesWithCategory(species: any) {
    const modelCategory = this.computeModelCategory(species);
    let daysToBloom = 0;
    if (modelCategory === 'flower') daysToBloom = 20;
    else if (modelCategory === 'vegetable') daysToBloom = 30;
    else if (modelCategory === 'tree') daysToBloom = 50;

    return {
      ...species,
      commonName: species.commonName
        ? species.commonName.charAt(0).toUpperCase() + species.commonName.slice(1)
        : species.commonName,
      modelCategory,
      daysToBloom,
    };
  }

  async create(createGardenDto: CreateGardenDto) {
    try {
      const { bloomDate, ...restDto } = createGardenDto;
      let parsedBloomDate: Date | undefined;

      if (bloomDate) {
        parsedBloomDate = new Date(bloomDate);
        const today = new Date();
        today.setHours(0, 0, 0, 0);
        if (parsedBloomDate < today) {
          throw new BadRequestException('Target bloom date cannot be in the past');
        }
      }

      return await this.db.garden.create({
        data: {
          description: restDto.description ?? '',
          timezone: restDto.timezone ?? undefined,
          ...restDto,
          bloomDate: parsedBloomDate,
        },
      });
    } catch (err: unknown) {
      // Narrow to PrismaClientKnownRequestError
      if (err instanceof Prisma.PrismaClientKnownRequestError) {
        if (err.code === 'P2003') {
          throw new BadRequestException(
            `Invalid ownerId: ${createGardenDto.ownerId} does not exist`,
          );
        }
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  findAll() {
    return this.db.garden.findMany();
  }

  findOne(id: number) {
    return this.db.garden.findUnique({
      where: {
        id: id,
      },
    });
  }

  findGardensByOwnerId(ownerId: number) {
    return this.db.garden.findMany({
      where: { ownerId },
      orderBy: { lastUpdated: 'desc' },
      include: {
        _count: { select: { plants: true } },
      },
    });
  }

  async findGardenForOwner(gardenId: number, ownerId: number) {
    const garden = await this.db.garden.findFirst({
      where: { id: gardenId, ownerId },
      include: {
        plants: {
          include: {
            species: true,
            soil: true,
          },
        },
      },
    });
    if (!garden) {
      throw new NotFoundException(
        `Garden ${gardenId} not found or not owned by user ${ownerId}`,
      );
    }

    return {
      ...garden,
      plants: garden.plants.map((plant) => ({
        ...plant,
        species: this.mapSpeciesWithCategory(plant.species),
      })),
    };
  }

  async update(id: number, updateGardenDto: UpdateGardenDto) {
    try {
      const { bloomDate, ...restDto } = updateGardenDto as any;
      let parsedBloomDate: Date | undefined;

      if (bloomDate) {
        parsedBloomDate = new Date(bloomDate);
        const today = new Date();
        today.setHours(0, 0, 0, 0);
        if (parsedBloomDate < today) {
          throw new BadRequestException('Target bloom date cannot be in the past');
        }
      }

      return await this.db.garden.update({
        where: { id },
        data: {
          ...restDto,
          ...(parsedBloomDate !== undefined && { bloomDate: parsedBloomDate }),
        },
      });
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Garden with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      return await this.db.garden.delete({
        where: {
          id: id,
        },
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Garden with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }
}
