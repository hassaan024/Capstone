import {
  BadRequestException,
  Injectable,
  NotFoundException,
  Logger,
} from '@nestjs/common';
import { CreateUserDto } from './dto/create-user.dto.js';
import { UpdateUserDto } from './dto/update-user.dto.js';
import { DatabaseService } from 'database/database.service';
import { Prisma } from '@prisma/client';

@Injectable()
export class UserService {
  private readonly logger = new Logger(UserService.name);

  constructor(private db: DatabaseService) {}

  async create(createUserDto: CreateUserDto) {
    try {
      const user = await this.db.user.create({
        data: {
          ...createUserDto,
          passwordHash: createUserDto.passwordHash ?? '',
        },
      });
      
      this.logger.log(`✅ User created - ID: ${user.id}, Email: ${user.email}, Name: ${user.displayName}`);
      
      return user;
    } catch (err: unknown) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2002'
      ) {
        this.logger.warn(`❌ User creation failed - Duplicate email: ${createUserDto.email}`);
        throw new BadRequestException('A user with this email already exists.');
      }
      this.logger.error(`❌ User creation failed: ${(err as Error).message}`);
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
      const user = await this.db.user.update({
        where: { id },
        data: updateUserDto,
      });
      
      this.logger.log(`✅ User updated - ID: ${id}, Email: ${user.email}, Updated fields: ${Object.keys(updateUserDto).join(', ')}`);
      
      return user;
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        this.logger.warn(`❌ User update failed - User not found: ${id}`);
        throw new NotFoundException(`User with id ${id} not found`);
      }
      this.logger.error(`❌ User update failed for ID ${id}: ${(err as Error).message}`);
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }

  async remove(id: number) {
    try {
      const user = await this.db.user.delete({
        where: {
          id: id,
        },
      });
      
      this.logger.warn(`🗑️  User deleted - ID: ${id}, Email: ${user.email}, Name: ${user.displayName}`);
      
      return user;
    } catch (err) {
      if (
        err instanceof Prisma.PrismaClientKnownRequestError &&
        err.code === 'P2025'
      ) {
        this.logger.warn(`❌ User deletion failed - User not found: ${id}`);
        throw new NotFoundException(`User with id ${id} not found`);
      }
      this.logger.error(`❌ User deletion failed for ID ${id}: ${(err as Error).message}`);
      throw new BadRequestException((err as Error).message ?? 'Unknown error');
    }
  }
}
