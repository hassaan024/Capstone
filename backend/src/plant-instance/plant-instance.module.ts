import { Module } from '@nestjs/common';
import { PlantInstanceService } from './plant-instance.service.js';
import { PlantInstanceController } from './plant-instance.controller.js';

@Module({
  controllers: [PlantInstanceController],
  providers: [PlantInstanceService],
})
export class PlantInstanceModule {}
