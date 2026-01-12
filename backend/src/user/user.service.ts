import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { CreateUserDto } from './dto/create-user.dto.js';
import { UpdateUserDto } from './dto/update-user.dto.js';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class UserService {
  constructor(private db: DatabaseService) {}

  async create(createUserDto: CreateUserDto) {
    try {
      return await this.db.user.create({
        data: {
          ...createUserDto,
          password: createUserDto.password ?? undefined,
        },
      });
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2002'
      ) {
        throw new BadRequestException('A user with this email already exists.');
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  findAll() {
    return this.db.user.findMany();
  }

  findOne(id: number) {
    return this.db.user.findUnique({
      where: {
        id: id,
      },
    });
  }

  async update(id: number, updateUserDto: UpdateUserDto) {
    try {
      return await this.db.user.update({
        where: { id },
        data: updateUserDto, // any subset of fields from CreateUserDto
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`User with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      return await this.db.user.delete({
        where: {
          id: id,
        },
      });
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        throw new NotFoundException(`User with id ${id} not found`);
      }
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }
}
