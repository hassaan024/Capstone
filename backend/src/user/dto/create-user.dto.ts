import { IsEmail, IsOptional, IsString } from 'class-validator';

export class CreateUserDto {
  @IsEmail()
  email: string;

  @IsString()
  displayName: string;

  @IsOptional()
  @IsString()
  passwordHash: string;
}
