import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { CreatePlantInstanceDto } from './dto/create-plant-instance.dto';
import { UpdatePlantInstanceDto } from './dto/update-plant-instance.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class PlantInstanceService {
  constructor(private readonly db: DatabaseService) {}

  async create(createPlantInstanceDto: CreatePlantInstanceDto) {
    try {
      return await this.db.plantInstance.create({
        data: {
          ...createPlantInstanceDto,
        },
      });
    } catch (err: unknown) {
      // P2003 = Foreign key constraint failed
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2003'
      ) {
        const field = err.meta?.constraint ?? 'foreign key';
        console.log(field);
        switch (field) {
          case 'PlantInstance_gardenId_fkey':
            throw new BadRequestException('invalid foreign key for gardenId');
          case 'PlantInstance_speciesId_fkey':
            throw new BadRequestException('invalid foreign key for speciesId');
          case 'PlantInstance_soilId_fkey':
            throw new BadRequestException('invalid foreign key for soilId');
          default:
            throw new BadRequestException('Invalid foreign key reference');
        }
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  findAll() {
    return this.db.plantInstance.findMany();
  }

  findOne(id: number) {
    return this.db.plantInstance.findUnique({
      where: {
        id: id,
      },
    });
  }

  async update(id: number, updatePlantInstanceDto: UpdatePlantInstanceDto) {
    try {
      return await this.db.plantInstance.update({
        where: { id },
        data: updatePlantInstanceDto,
      });
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Plant_Instance with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      return await this.db.plantInstance.delete({
        where: {
          id: id,
        },
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Plant_Instance with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }
}
