import { Controller, Get, Post, Body, Query, ParseIntPipe, Param } from '@nestjs/common';
import { PredictionService } from './prediction.service';
import { GenerateTimelineDto } from './dto/generate-timeline.dto';

@Controller('prediction')
export class PredictionController {
  constructor(private readonly predictionService: PredictionService) {}

  @Post(':plantInstanceId')
  predict(
    @Param('plantInstanceId', ParseIntPipe) plantInstanceId: number,
    @Query('daysAhead', ParseIntPipe) daysAhead: number,
  ) {
    return this.predictionService.predict(plantInstanceId, daysAhead);
  }

  /**
   * Generates the full growth timeline for every plant in a garden.
   * Expects a bloomDate (the in-game date when all plants should be fully grown).
   * Calculates each plant's planted date, runs stepped ML predictions at
   * 7-day intervals, stores each snapshot in PlantHistory, and returns the
   * complete timeline for Unreal's slider.
   */
  @Post('garden/:gardenId/timeline')
  generateTimeline(
    @Param('gardenId', ParseIntPipe) gardenId: number,
    @Body() dto: GenerateTimelineDto,
  ) {
    return this.predictionService.generateTimeline(gardenId, new Date(dto.bloomDate));
  }

  /**
   * Returns an estimated bloom date for a species given a planted date.
   * No ML call — fast formula-based estimate.
   * Use this to show "~X months to full bloom" in the UI when browsing species.
   *
   * plantedDate defaults to today if not provided.
   */
  @Get('bloom-estimate/:speciesId')
  bloomEstimate(
    @Param('speciesId', ParseIntPipe) speciesId: number,
    @Query('plantedDate') plantedDate?: string,
  ) {
    const date = plantedDate ? new Date(plantedDate) : new Date();
    return this.predictionService.bloomEstimate(speciesId, date);
  }

  /**
   * Seeds the three Louisiana demo species (Zinnia, Creole Tomato, Crape Myrtle)
   * into the database with accurate hardcoded data.
   * Safe to call multiple times. Returns the speciesId for each so Unreal
   * knows which IDs to use when creating demo plant instances.
   */
  @Post('demo/seed')
  seedDemoSpecies() {
    return this.predictionService.seedDemoSpecies();
  }
}
