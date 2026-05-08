import {
  Injectable,
  BadRequestException,
  NotFoundException,
  forwardRef,
  Inject,
} from '@nestjs/common';
import { CreateGardenDto } from './dto/create-garden.dto';
import { UpdateGardenDto } from './dto/update-garden.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';
import { computeBloomDays } from 'utils/plant-growth';
import { GardenScheduler } from './garden.scheduler';

@Injectable()
export class GardenService {
  constructor(
    private readonly db: DatabaseService,
    @Inject(forwardRef(() => GardenScheduler))
    private readonly scheduler: GardenScheduler,
  ) {}

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

    if (typeStr.includes('tree')) return 'tree';

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

    if (species.scientificName?.includes('Malus')) return 'tree';

    return 'flower';
  }

  private mapSpeciesWithCategory(species: any) {
    const modelCategory = this.computeModelCategory(species);
    const bloomDays = species.bloomDays ?? computeBloomDays(species);

    return {
      ...species,
      commonName: species.commonName
        ? species.commonName.charAt(0).toUpperCase() + species.commonName.slice(1)
        : species.commonName,
      modelCategory,
      bloomDays,
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

      const updated = await this.db.garden.update({
        where: { id },
        data: {
          ...restDto,
          ...(parsedBloomDate !== undefined && { bloomDate: parsedBloomDate }),
        },
      });

      return updated;
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

  async getPlantingAlerts(userId: number) {
    const gardens = await this.db.garden.findMany({
      where: { ownerId: userId },
      include: {
        plants: {
          include: {
            species: true,
          },
        },
      },
    });

    const alerts = [];
    const now = new Date();
    // Use start of today to ensure full 7 days inclusive check
    const today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
    const sevenDaysFromNow = new Date(today.getTime() + 7 * 24 * 60 * 60 * 1000);

    for (const garden of gardens) {
      for (const plant of garden.plants) {
        let plantedDate: Date | null = null;
        if (plant.plantedDate) {
          plantedDate = new Date(plant.plantedDate);
        } else if (plant.bloomDate && plant.species.bloomDays) {
          const bloomMs = new Date(plant.bloomDate).getTime();
          plantedDate = new Date(bloomMs - plant.species.bloomDays * 24 * 60 * 60 * 1000);
        } else if (garden.bloomDate && plant.species.bloomDays) {
          const bloomMs = new Date(garden.bloomDate).getTime();
          plantedDate = new Date(bloomMs - plant.species.bloomDays * 24 * 60 * 60 * 1000);
        }

        if (plantedDate) {
          // Check if plantedDate is between today and 7 days from now (inclusive)
          // We also include past dates up to 1 day if they missed it, or just strictly future?
          // The user said: "if a plant date is approaching... notify a user 7 days before"
          // Let's just do: today <= plantedDate <= 7 days from now
          const pDateMidnight = new Date(plantedDate.getFullYear(), plantedDate.getMonth(), plantedDate.getDate());
          if (pDateMidnight >= today && pDateMidnight <= sevenDaysFromNow) {
            
            // To ensure modelCategory is attached, we can optionally reuse the computeModelCategory logic
            // from SpeciesService, but frontend also has mapPlantToVisualCategory as fallback.
            // Let's just return what we have.
            alerts.push({
              plantInstanceId: plant.id,
              gardenId: garden.id,
              gardenName: garden.name,
              species: plant.species,
              plantedDate: plantedDate.toISOString(),
              bloomDate: plant.bloomDate?.toISOString() ?? null,
              notificationDate: now.toISOString(),
            });
          }
        }
      }
    }

    // Sort alerts by plantedDate ascending (most urgent first)
    alerts.sort((a, b) => new Date(a.plantedDate).getTime() - new Date(b.plantedDate).getTime());

    return alerts;
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
