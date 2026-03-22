import { Controller, Get, Param, Post, Query } from '@nestjs/common';
import { PerenualService } from './perenual.service';
import { SearchPlantsQueryDto } from './dto/search-plants-query.dto';
import { QueryResult } from 'typeorm';
import { PlantIdParamDto } from './dto/plant-id-param.dto';

@Controller('perenual')
export class PerenualController {
  constructor(private readonly service: PerenualService) {}

  // search plants from perenual api
  @Get('search')
  search(@Query() queryDto: SearchPlantsQueryDto) {
    return this.service.searchPlants(queryDto);
  }

  // get full plant details from perenual api
  @Get('details/:id')
  details(@Param() params: PlantIdParamDto) {
    return this.service.getPlantDetails(Number(params.id));
  }

  // // import plant from perenual -> database
  // @Post('import-to-db/:id')
  // import(@Param() params: PlantIdParamDto) {
  //   return this.service.importSpecies(Number(params.id));
  // }
}
