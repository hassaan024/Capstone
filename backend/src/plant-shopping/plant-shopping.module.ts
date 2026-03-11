import { Module } from '@nestjs/common';
import { PlantShoppingController } from './plant-shopping.controller.js';
import { PlantShoppingService } from './plant-shopping.service.js';

@Module({
  controllers: [PlantShoppingController],
  providers: [PlantShoppingService],
})
export class PlantShoppingModule {}
