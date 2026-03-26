import { IsInt, IsOptional, IsDateString, IsNumber, Min } from 'class-validator';

export class CreatePlantHistoryDto {
  @IsInt()
  plantId: number;

  @IsDateString()
  date: string; // ISO string (e.g., "2026-03-21T15:00:00Z")

  @IsOptional()
  @IsNumber()
  @Min(0)
  heightCm?: number; // plant height in cm

  @IsOptional()
  @IsNumber()
  predictedValue?: number; // optional predicted value (e.g., growth prediction)
}