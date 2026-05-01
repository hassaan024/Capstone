import { Module } from '@nestjs/common';
import { PredictionService } from './prediction.service';
import { PredictionController } from './prediction.controller';
import { DatabaseModule } from 'database/database.module';
import { WeatherModule } from 'weather/weather.module';

@Module({
  imports: [DatabaseModule, WeatherModule],
  controllers: [PredictionController],
  providers: [PredictionService],
})
export class PredictionModule {}