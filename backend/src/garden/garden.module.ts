import { Module } from '@nestjs/common';
import { GardenService } from './garden.service';
import { GardenController } from './garden.controller';
import { DatabaseModule } from 'database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [GardenController],
  providers: [GardenService],
})
export class GardenModule {}
