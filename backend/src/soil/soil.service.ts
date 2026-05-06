import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { CreateSoilDto } from './dto/create-soil.dto.js';
import { UpdateSoilDto } from './dto/update-soil.dto.js';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class SoilService {
  constructor(private readonly db: DatabaseService) {}

  async create(createSoilDto: CreateSoilDto) {
    try {
      return await this.db.soil.create({
        data: {
          ...createSoilDto,
        },
      });
    } catch (err: unknown) {
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  findAll() {
    return this.db.soil.findMany();
  }

  findOne(id: number) {
    return this.db.soil.findUnique({
      where: {
        id: id,
      },
    });
  }

  async update(id: number, updateSoilDto: UpdateSoilDto) {
    try {
      return await this.db.soil.update({
        where: { id },
        data: updateSoilDto,
      });
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Soil with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      return await this.db.soil.delete({
        where: {
          id: id,
        },
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`Soil with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }
}
