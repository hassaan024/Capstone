import { Module } from '@nestjs/common';
import { PlantHistoryService } from './plant-history.service';
import { PlantHistoryController } from './plant-history.controller';

@Module({
  providers: [PlantHistoryService],
  controllers: [PlantHistoryController]
})
export class PlantHistoryModule {}
