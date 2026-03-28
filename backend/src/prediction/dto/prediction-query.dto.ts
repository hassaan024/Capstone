import { IsDateString, IsInt, IsNumber } from "class-validator";

export class PredictionQueryDto {
  @IsInt()
  plantId: number;

  @IsDateString()
  start: string;

  @IsDateString()
  end: string;

  @IsNumber()
  longitude: number;

  @IsNumber()
  latitude: number;
}