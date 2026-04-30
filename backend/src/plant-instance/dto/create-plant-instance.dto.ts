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

  @IsNumber()
  locationX: number;

  @IsNumber()
  locationY: number;

  @IsNumber()
  locationZ: number;

  @IsNumber()
  rotationPitch: number;

  @IsNumber()
  rotationYaw: number;

  @IsNumber()
  rotationRoll: number;

  @IsNumber()
  scaleX: number;

  @IsNumber()
  scaleY: number;

  @IsNumber()
  scaleZ: number;

  @IsOptional()
  @IsNumber()
  heightCm?: number;

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
  @IsDate()
  @Type(() => Date)
  plantedDate?: Date;   // defaults to now() if not provided

  @IsOptional()
  @IsDate()
  @Type(() => Date)
  currentGameDate?: Date;  // defaults to now() if not provided

  @IsOptional()
  @IsString()
  notes?: string;
}