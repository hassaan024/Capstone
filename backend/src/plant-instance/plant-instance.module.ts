import { Module } from '@nestjs/common';
import { PlantInstanceService } from './plant-instance.service';
import { PlantInstanceController } from './plant-instance.controller';

@Module({
  controllers: [PlantInstanceController],
  providers: [PlantInstanceService],
})
export class PlantInstanceModule {}
