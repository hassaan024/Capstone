import { IsOptional, IsString } from 'class-validator';

export class SearchPlantsQueryDto {
  @IsString()
  query: string; // search term (e.g. "tomato")

  @IsOptional()
  @IsString()
  page?: string; // optional page number
}