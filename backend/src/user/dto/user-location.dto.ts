import { IsNumber, IsOptional, IsString } from 'class-validator';

export class UserLocationDto {
  // stuff for the web users
  @IsOptional()
  @IsNumber()
  latitude?: number;

  @IsOptional()
  @IsNumber()
  longitude?: number;

  @IsOptional()
  @IsString()
  updatedAt?: Date;
}
