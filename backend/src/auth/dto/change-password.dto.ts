import { IsNotEmpty, IsString, IsNumber } from 'class-validator';

export class ChangePasswordDto {
  @IsNotEmpty({ message: 'User ID is required' })
  @IsNumber()
  userId: number;

  @IsNotEmpty({ message: 'Current password is required' })
  @IsString()
  oldPassword: string;

  @IsNotEmpty({ message: 'New password is required' })
  @IsString()
  newPassword: string;
}
