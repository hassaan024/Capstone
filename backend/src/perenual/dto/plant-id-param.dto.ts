import { IsNumberString } from 'class-validator';

export class PlantIdParamDto {
  @IsNumberString()
  id: string; // Perenual species ID (e.g. 1056)
}