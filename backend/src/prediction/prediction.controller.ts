import { Controller, Get, Post, Body, Query } from '@nestjs/common';
import { PredictionService } from './prediction.service';
import { PredictionQueryDto } from './dto/prediction-query.dto';
import { BatchPredictionDto } from './dto/batch-prediction.dto';

@Controller('prediction')
export class PredictionController {
  constructor(private readonly predictionService: PredictionService) {}

  // PredictionQueryDto
  @Get()
  getPrediction(@Query() query: any) {
    return this.predictionService.getPrediction(query);
  }

  // BatchPredictionDto
  @Post('batch')
  getBatchPrediction(@Body() batchQuery: any) {
    return this.predictionService.getBatchPrediction(batchQuery);
  }
}
