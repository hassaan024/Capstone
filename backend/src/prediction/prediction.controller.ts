import { Controller, Get, Post, Body, Query, ParseIntPipe, Param } from '@nestjs/common';
import { PredictionService } from './prediction.service';
import { PredictionQueryDto } from './dto/prediction-query.dto';
import { BatchPredictionDto } from './dto/batch-prediction.dto';

@Controller('prediction')
export class PredictionController {
  constructor(private readonly predictionService: PredictionService) {}

  // PredictionQueryDto
  @Post(':plantInstanceId')
  predict(
    @Param('plantInstanceId', ParseIntPipe) plantInstanceId: number,
    @Query('daysAhead', ParseIntPipe) daysAhead: number,
  ) {
    return this.predictionService.predict(plantInstanceId, daysAhead);
  }
}
