import {
  IsNumber,
  IsOptional,
  IsString,
  IsBoolean,
  IsArray,
  ValidateNested,
} from 'class-validator';
import { Type } from 'class-transformer';
import { ImageUrlsDto } from './image-urls.dto';

export class DimensionDto {
  type?: string;       // e.g., "height", "spread", nullable
  minValue: number;    // e.g., 3
  maxValue: number;    // e.g., 6
  unit: string;        // e.g., "feet"
}

export class CreateSpeciesDto {
  // ---------- Core ----------
  @IsString()
  commonName: string;

  @IsString()
  scientificName: string;

  @IsNumber()
  growthRate: number;

  @IsOptional()
  dimensions: DimensionDto[];

  @IsOptional()
  @IsString()
  cycle?: string;

  @IsOptional()
  @IsString()
  type?: string;

  @IsOptional()
  @IsString()
  maintenance?: string;

  @IsOptional()
  @IsString()
  careLevel?: string;

  @IsOptional()
  @IsNumber()
  avgHoursSun?: number;

  @IsOptional()
  @IsNumber()
  minTemp?: number;

  @IsOptional()
  @IsNumber()
  maxTemp?: number;

  @IsOptional()
  @IsNumber()
  minHeight?: number;

  @IsOptional()
  @IsNumber()
  maxHeight?: number;

  @IsOptional()
  @IsString()
  wateringFreq?: string;

  @IsOptional()
  @IsNumber()
  wateringMinDays?: number;

  @IsOptional()
  @IsNumber()
  wateringMaxDays?: number;

  // ---------- Taxonomy ----------
  @IsOptional()
  @IsArray()
  otherNames?: string[];

  @IsOptional()
  @IsString()
  family?: string;

  @IsOptional()
  @IsString()
  genus?: string;

  @IsOptional()
  @IsString()
  speciesEpithet?: string;

  @IsOptional()
  @IsArray()
  origin?: string[];

  // ---------- Reproduction ----------
  @IsOptional()
  @IsBoolean()
  flowers?: boolean;

  @IsOptional()
  @IsString()
  floweringSeason?: string;

  @IsOptional()
  @IsBoolean()
  fruits?: boolean;

  @IsOptional()
  @IsBoolean()
  edibleFruit?: boolean;

  @IsOptional()
  @IsString()
  harvestSeason?: string;

  @IsOptional()
  @IsBoolean()
  leaf?: boolean;

  @IsOptional()
  @IsBoolean()
  edibleLeaf?: boolean;

  @IsOptional()
  @IsBoolean()
  cuisine?: boolean;

  @IsOptional()
  @IsBoolean()
  medicinal?: boolean;

  // ---------- Environment ----------
  @IsOptional()
  @IsBoolean()
  droughtTolerant?: boolean;

  @IsOptional()
  @IsBoolean()
  saltTolerant?: boolean;

  @IsOptional()
  @IsBoolean()
  tropical?: boolean;

  @IsOptional()
  @IsBoolean()
  indoor?: boolean;

  @IsOptional()
  @IsBoolean()
  thorny?: boolean;

  @IsOptional()
  @IsBoolean()
  invasive?: boolean;

  // ---------- Care ----------
  @IsOptional()
  @IsArray()
  pruningMonths?: string[];

  @IsOptional()
  @IsNumber()
  pruningFrequency?: number;

  @IsOptional()
  @IsString()
  pruningInterval?: string;

  // ---------- Visual ----------
  @IsOptional()
  plantAnatomy?: any; // keep flexible (complex structure)

  @IsOptional()
  @ValidateNested()
  @Type(() => ImageUrlsDto)
  imgSrcUrls?: ImageUrlsDto;

  // ---------- Misc ----------
  @IsOptional()
  @IsString()
  notes?: string;
}