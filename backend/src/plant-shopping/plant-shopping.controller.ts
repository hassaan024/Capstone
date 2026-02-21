import { Controller, Get, Param, Logger, BadRequestException } from '@nestjs/common';
import { PlantShoppingService } from './plant-shopping.service.js';

@Controller('plant-shopping')
export class PlantShoppingController {
  private readonly logger = new Logger(PlantShoppingController.name);

  constructor(private readonly plantShoppingService: PlantShoppingService) {}

  @Get('links/:plantName')
  getShoppingLinks(@Param('plantName') plantName: string) {
    if (!plantName || !plantName.trim()) {
      throw new BadRequestException('Plant name is required');
    }
    this.logger.log(`Generating shopping links for: ${plantName}`);
    return this.plantShoppingService.getShoppingLinks(plantName);
  }

  @Get('price-history/:plantId')
  getPriceHistory(@Param('plantId') plantId: string) {
    const numericId = parseInt(plantId, 10);
    if (isNaN(numericId)) {
      throw new BadRequestException('Invalid plant ID');
    }
    this.logger.log(`Generating price history for plant ID: ${numericId}`);
    return this.plantShoppingService.getPriceHistory(numericId);
  }
}
