import { IsArray, IsDateString, IsNumber } from 'class-validator';

export class BatchPredictionDto {
  @IsArray()
  plantIds: number[];

  @IsDateString()
  start: string;

  @IsDateString()
  end: string;

  @IsNumber()
  lat: number;

  @IsNumber()
  lon: number;
}