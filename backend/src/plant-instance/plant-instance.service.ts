import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { CreatePlantInstanceDto } from './dto/create-plant-instance.dto';
import { UpdatePlantInstanceDto } from './dto/update-plant-instance.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';
import { getSoilDefaults } from 'utils/soil-defaults';
import { GardenScheduler } from '../garden/garden.scheduler';

@Injectable()
export class PlantInstanceService {
  constructor(
    private readonly db: DatabaseService,
    private readonly scheduler: GardenScheduler,
  ) {}

  async create(createPlantInstanceDto: CreatePlantInstanceDto) {
    try {
      // Auto-create soil from type + defaults
      const defaults = getSoilDefaults(createPlantInstanceDto.soilType);
      const soil = await this.db.soil.create({
        data: {
          type: createPlantInstanceDto.soilType,
          ...defaults,
        },
      });

      // Create plant instance linked to that soil
      const instance = await this.db.plantInstance.create({
        data: {
          gardenId: createPlantInstanceDto.gardenId,
          speciesId: createPlantInstanceDto.speciesId,
          soilId: soil.id,
          locationX: createPlantInstanceDto.locationX,
          locationY: createPlantInstanceDto.locationY,
          locationZ: createPlantInstanceDto.locationZ,
          rotationPitch: createPlantInstanceDto.rotationPitch,
          rotationYaw: createPlantInstanceDto.rotationYaw,
          rotationRoll: createPlantInstanceDto.rotationRoll,
          scaleX: createPlantInstanceDto.scaleX,
          scaleY: createPlantInstanceDto.scaleY,
          scaleZ: createPlantInstanceDto.scaleZ,
          heightCm: createPlantInstanceDto.heightCm ?? null,
          healthStatus: createPlantInstanceDto.healthStatus ?? null,
          growthStage: createPlantInstanceDto.growthStage ?? null,
          bloomState: createPlantInstanceDto.bloomState ?? false,
          lastWatered: createPlantInstanceDto.lastWatered ?? null,
          plantedDate: createPlantInstanceDto.plantedDate ?? new Date(),
          currentGameDate: createPlantInstanceDto.currentGameDate ?? new Date(),
          notes: createPlantInstanceDto.notes ?? null,
        },
        include: { species: { select: { commonName: true, scientificName: true, type: true } } },
      });

      // Fire email check in the background — never blocks the response
      void this.scheduler.triggerAlertEmailIfNeeded(createPlantInstanceDto.gardenId);

      return instance;
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2003'
      ) {
        const field = err.meta?.constraint ?? 'foreign key';
        switch (field) {
          case 'PlantInstance_gardenId_fkey':
            throw new BadRequestException('invalid foreign key for gardenId');
          case 'PlantInstance_speciesId_fkey':
            throw new BadRequestException('invalid foreign key for speciesId');
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
