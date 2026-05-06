import { IsEnum, IsNumber, IsOptional } from 'class-validator';
import { SoilType } from 'enums/table_enums';

export class CreateSoilDto {
  // Required floats
  @IsNumber()
  nitrogen: number;

  @IsNumber()
  phosphorus: number;

  @IsNumber()
  potassium: number;

  @IsNumber()
  pH: number;

  // Optional float
  @IsOptional()
  @IsNumber()
  organicPercentage?: number;

  // Enum
  @IsEnum(SoilType)
  type: SoilType;
}
