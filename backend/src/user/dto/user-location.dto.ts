import { IsNumber, IsOptional, IsString } from 'class-validator';

export class UserLocationDto {
  // stuff for the web users
  @IsOptional()
  @IsNumber({ maxDecimalPlaces: 8 })
  latitude?: number;

  @IsOptional()
  @IsNumber({ maxDecimalPlaces: 8 })
  longitude?: number;

  @IsOptional()
  @IsString()
  updatedAt?: Date;
}
