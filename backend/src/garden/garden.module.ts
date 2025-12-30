import { Module } from '@nestjs/common';
import { GardenService } from './garden.service.js';
import { GardenController } from './garden.controller.js';

@Module({
  controllers: [GardenController],
  providers: [GardenService],
})
export class GardenModule {}
