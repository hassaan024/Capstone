// src/species/dto/image-urls.dto.ts
import { IsOptional, IsUrl } from 'class-validator';

export class ImageUrlsDto {
  [key: string]: string | null

  @IsOptional()
  @IsUrl()
  original?: string | null;

  @IsOptional()
  @IsUrl()
  regular?: string | null;

  @IsOptional()
  @IsUrl()
  medium?: string | null;

  @IsOptional()
  @IsUrl()
  small?: string | null;

  @IsOptional()
  @IsUrl()
  thumbnail?: string | null;
}