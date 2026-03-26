import { IsString } from 'class-validator';

export class GoogleOAuthDto {
  @IsString()
  code: string;
}
