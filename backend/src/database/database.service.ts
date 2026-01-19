// import { PrismaPostgresAdapter } from '@prisma/adapter-ppg';
import { Injectable, OnModuleInit, OnModuleDestroy } from '@nestjs/common';
import { PrismaClient, Prisma } from '@prisma/client';

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
  deleteMany: (args?: any) => Promise<Prisma.BatchPayload>;
  findMany: (args?: any) => Promise<any[]>;
};

@Injectable()
export class DatabaseService
  extends PrismaClient
  implements OnModuleInit, OnModuleDestroy
{
  private readonly tableMap: TableMap;

  constructor() {
    const connection_url: string | undefined = process.env.DATABASE_URL;

    if (!connection_url) {
      throw new Error('DATABASE_URL (connection_url) is undefined');
    }

    console.log(`USING THIS CONNECTION URL ${connection_url}`);

    super();
    this.tableMap = {
      User: this.user,
      Garden: this.garden,
      PlantInstance: this.plantInstance,
      Species: this.species,
      Soil: this.soil,
    };
  }

  // connect to the database when you init
  async onModuleInit() {
    try {
      await this.$connect();
      console.log(`Connected to DB Successfuly`);
    } catch (err) {
      console.error('Failed to connect to PostgressDB:', err);
    } finally {
      await this.$disconnect();
    }
    await this.printDatabase();
  }
  async onModuleDestroy() {
    await this.$disconnect();
    console.log(`Disconnected from DB Successfuly`);
  }

  getTable(table_name: PrismaTableName): DatabaseTableOperation {
    const table = this.tableMap[table_name];
    if (!table) throw new Error(`Table ${table_name} not found`);
    return table;
  }
  // clear a single table (returns number of rows cleared)
  async clearTable(table_name: PrismaTableName): Promise<number> {
    const table = this.getTable(table_name);
    let rows_deleted: Prisma.BatchPayload;

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
    console.log('Starting to clear the database...');
    for (const tableName of deletionOrder) {
      try {
        const count = await this.clearTable(tableName);
        console.log(`Deleted ${count} rows from ${tableName}`);
      } catch (err) {
        console.error(`Failed to clear ${tableName}:`, err);
      }
    }
    console.log('CLEARED THE DATABASE!');
  }

  async printTable(tableName: PrismaTableName) {
    const table = this.getTable(tableName);

    try {
      const rows = await table.findMany();
      console.log(`\n=== TABLE: ${tableName} ===`);
      console.log(rows);
      console.log(`=== END ${tableName} ===\n`);
      // eslint-disable-next-line @typescript-eslint/no-unsafe-return
      return rows;
    } catch (err) {
      console.error(`Failed to print table ${tableName}:`, err);
      return [];
    }
  }

  async printDatabase() {
    console.log('\n========== DATABASE START ==========');

    for (const tableName of deletionOrder) {
      await this.printTable(tableName);
    }

    console.log('=========== DATABASE END ===========\n');
  }
}
