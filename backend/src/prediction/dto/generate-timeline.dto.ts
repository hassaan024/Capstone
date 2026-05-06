import { IsDateString } from 'class-validator';

export class GenerateTimelineDto {
  @IsDateString()
  bloomDate: string;
}
