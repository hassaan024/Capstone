import {
  IsEmail,
  IsOptional,
  IsString,
  IsBoolean,
  IsInt,
} from 'class-validator';

export class CreateUserDto {
  @IsEmail()
  email: string;

  @IsString()
  displayName: string;

  // @IsBoolean()
  // confirmedName: boolean;

  @IsOptional()
  @IsString()
  passwordHash?: string;

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
}
