import { Type } from 'class-transformer';
import {
  IsDate,
  IsEnum,
  IsInt,
  IsNumber,
  IsOptional,
  IsString,
} from 'class-validator';
import { HealthStatus } from 'enums/table_enums';

export class CreatePlantInstanceDto {
  @IsInt()
  gardenId: number;

  @IsInt()
  speciesId: number;

  @IsInt()
  soilId: number;

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
  @IsInt()
  ageDays?: number;

  @IsOptional()
  @IsEnum(HealthStatus, {
    message: 'healthStatus must be a valid HealthStatus enum value',
  })
  healthStatus?: HealthStatus;

  @IsOptional()
  @IsDate()
  @Type(() => Date)
  lastWatered?: Date;

  @IsOptional()
  @IsDate()
  @Type(() => Date)
  plantedDate?: Date;

  @IsOptional()
  @IsString()
  notes?: string;
}
