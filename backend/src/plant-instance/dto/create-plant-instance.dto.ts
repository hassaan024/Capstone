import { Type } from 'class-transformer';
import {
  IsBoolean, IsDate, IsEnum, IsInt, IsNumber, IsOptional, IsString,
} from 'class-validator';
import { HealthStatus, SoilType, GrowthStage } from 'enums/table_enums';

export class CreatePlantInstanceDto {
  @IsInt()
  gardenId: number;

  @IsInt()
  speciesId: number;

  @IsEnum(SoilType)
  soilType: SoilType;

  @IsOptional()
  @IsNumber()
  heightCm?: number;

  @IsOptional()
  @IsInt()
  ageDays?: number;

  @IsOptional()
  @IsEnum(HealthStatus)
  healthStatus?: HealthStatus;

  @IsOptional()
  @IsEnum(GrowthStage)
  growthStage?: GrowthStage;

  @IsOptional()
  @IsBoolean()
  bloomState?: boolean;

  @IsOptional()
  @IsDate()
  @Type(() => Date)
  lastWatered?: Date;

  @IsOptional()
  @IsString()
  notes?: string;
}