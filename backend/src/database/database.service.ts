import { PrismaClient } from '@generated/client';
import { BatchPayload } from '@generated/internal/prismaNamespace';
import { Injectable, OnModuleInit, OnModuleDestroy } from '@nestjs/common';
import { PrismaPg } from '@prisma/adapter-pg';

type PrismaTableName = 'User' | 'Garden' | 'PlantInstance' | 'Species' | 'Soil';

const deletionOrder: PrismaTableName[] = [
  'PlantInstance',
  'Garden',
  'User',
  'Species',
  'Soil',
];

type TableMap = {
  User: PrismaClient['user'];
  Garden: PrismaClient['garden'];
  PlantInstance: PrismaClient['plantInstance'];
  Species: PrismaClient['species'];
  Soil: PrismaClient['soil'];
};

type DatabaseTableOperation = {
  deleteMany: (args?: any) => Promise<BatchPayload>;
};

@Injectable()
export class DatabaseService
  extends PrismaClient
  implements OnModuleInit, OnModuleDestroy
{
  private readonly tableMap: TableMap;

  constructor() {
    // Use the Postgres adapter so Prisma Client can execute queries without a Rust engine
    const adapter = new PrismaPg({ url: process.env.DATABASE_URL });
    super({ adapter });
    this.tableMap = {
      User: this.user,
      Garden: this.garden,
      PlantInstance: this.plantInstance,
      Species: this.species,
      Soil: this.soil,
    };
  }

  getTable(table_name: PrismaTableName): DatabaseTableOperation {
    const table = this.tableMap[table_name];
    if (!table) throw new Error(`Table ${table_name} not found`);
    return table;
  }
  // clear a single table (returns number of rows cleared)
  async clearTable(table_name: PrismaTableName): Promise<number> {
    const table = this.getTable(table_name);
    let rows_deleted: BatchPayload;

    try {
      rows_deleted = await table.deleteMany();
      console.log(`CLEARED ${table_name} TABLE.`);
    } catch (err) {
      console.error(`Failed to clear table ${table_name}:`, err);
      rows_deleted = { count: 0 };
    }
    return rows_deleted.count;
  }

  // clear all tables
  async clearDatabase() {
    for (const tableName of deletionOrder) {
      const table = this.getTable(tableName);
      await table.deleteMany();
    }
    console.log('CLEARED THE DATABASE!');
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
