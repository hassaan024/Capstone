import { IsInt, IsNumber, IsOptional, IsString } from 'class-validator';

export class CreateGardenDto {
  @IsInt()
  ownerId: number;

  @IsString()
  name: string;

  @IsOptional()
  @IsString()
  description: string;

  @IsNumber()
  latitude: number;

  @IsNumber()
  longitude: number;

  @IsOptional()
  @IsString()
  timezone: string;

  @IsOptional()
  @IsString()
  bloomDate?: string;

  @IsOptional()
  @IsString()
  paintMaskData?: string;
}
