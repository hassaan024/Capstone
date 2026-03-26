import {
  IsEmail,
  IsOptional,
  IsString,
  IsBoolean,
  IsNumber,
} from 'class-validator';

export class CreateUserDto {
  @IsEmail()
  email: string;

  @IsString()
  displayName: string;

  @IsOptional()
  @IsString()
  passwordHash?: string;

  // stuff for the web users
  @IsOptional()
  @IsNumber()
  latitude?: number;

  @IsOptional()
  @IsNumber()
  longitude?: number;

  @IsOptional()
  @IsString()
  timezone?: string;

  // Google OAuth fields
  @IsOptional()
  @IsString()
  googleId?: string;

  @IsOptional()
  @IsBoolean()
  verifiedEmail?: boolean;

  @IsOptional()
  @IsString()
  googleDisplayName?: string;

  @IsOptional()
  @IsString()
  givenName?: string;

  @IsOptional()
  @IsString()
  familyName?: string;

  @IsOptional()
  @IsString()
  picture?: string;

  @IsOptional()
  @IsString()
  updatedAt?: Date;
}
