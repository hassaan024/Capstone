import { Injectable, Logger } from '@nestjs/common';
import { PredictionQueryDto } from './dto/prediction-query.dto';
import { BatchPredictionDto } from './dto/batch-prediction.dto';

@Injectable()
export class PredictionService {
  private readonly logger = new Logger(PredictionService.name);

  // PredictionQueryDto
  getPrediction(query: any) {
    this.logger.log("Making prediction for 1 query")
    return "Making prediction for 1 query";
  }
  
  // BatchPredictionDto
  getBatchPrediction(batchQuery: any) {
    this.logger.log("Making prediction for batch query")
    return "Making prediction for batch query";
  }
}
