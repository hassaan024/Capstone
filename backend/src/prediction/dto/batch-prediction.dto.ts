import { IsArray, IsDateString, IsNumber } from 'class-validator';

export class BatchPredictionDto {
  @IsArray()
  plantIds: number[];

  @IsDateString()
  start: string;

  @IsDateString()
  end: string;

  @IsNumber()
  longitude: number;

  @IsNumber()
  latitude: number;
}