import { IsNumber, IsOptional, IsString, IsUrl } from 'class-validator';

export class CreateSpeciesDto {
  // Required, unique
  @IsString()
  commonName: string;

  // Required, unique
  @IsString()
  scientificName: string;

  // Required float
  @IsNumber()
  growthRate: number;

  // Optional floats
  @IsOptional()
  @IsNumber()
  bloomRate?: number;

  @IsOptional()
  @IsNumber()
  witherRate?: number;

  @IsOptional()
  @IsNumber()
  minTemp?: number;

  @IsOptional()
  @IsNumber()
  maxTemp?: number;

  @IsOptional()
  @IsNumber()
  minHumidity?: number;

  @IsOptional()
  @IsNumber()
  maxHumidity?: number;

  @IsOptional()
  @IsNumber()
  avgHoursSun?: number;

  @IsOptional()
  @IsString()
  @IsUrl()
  imgSrcUrl?: string;

  @IsOptional()
  @IsString()
  notes?: string;
}
