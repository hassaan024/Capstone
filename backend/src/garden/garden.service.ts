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

  async create(createGardenDto: CreateGardenDto) {
    try {
      return await this.db.garden.create({
        data: {
          description: createGardenDto.description ?? '',
          timezone: createGardenDto.timezone ?? undefined,
          ...createGardenDto,
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
    return garden;
  }

  async update(id: number, updateGardenDto: UpdateGardenDto) {
    try {
      return await this.db.garden.update({
        where: { id },
        data: updateGardenDto,
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
