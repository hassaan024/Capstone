import {
  Injectable,
  BadRequestException,
  NotFoundException,
} from '@nestjs/common';
import { CreateSpeciesDto } from './dto/create-species.dto';
import { UpdateSpeciesDto } from './dto/update-species.dto';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class SpeciesService {
  constructor(private readonly db: DatabaseService) {}

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
