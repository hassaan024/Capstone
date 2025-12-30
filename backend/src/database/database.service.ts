import { PrismaClient } from '@generated/client';
import { Injectable, OnModuleInit, OnModuleDestroy } from '@nestjs/common';
import { PrismaPg } from '@prisma/adapter-pg';

@Injectable()
export class DatabaseService
  extends PrismaClient
  implements OnModuleInit, OnModuleDestroy
{
  constructor() {
    // Use the Postgres adapter so Prisma Client can execute queries without a Rust engine
    const adapter = new PrismaPg({ url: process.env.DATABASE_URL });
    super({ adapter });
  }
  // connect to the database when you init
  async onModuleInit() {
    await this.$connect();
    console.log(`Connected to DB Successfuly`);
  }
  async onModuleDestroy() {
    await this.$disconnect();
    console.log(`Disconnected from DB Successfuly`);
  }
}
