import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  Query,
} from '@nestjs/common';
import { SpeciesService } from './species.service.js';
import { CreateSpeciesDto } from './dto/create-species.dto.js';
import { UpdateSpeciesDto } from './dto/update-species.dto.js';

@Controller('species')
export class SpeciesController {
  constructor(private readonly speciesService: SpeciesService) {}

  @Get('saved')
  getSaved(
    @Query('userId') userId: string,
    @Query('gardenId') gardenId?: string,
  ) {
    if (gardenId) {
      return this.speciesService.getSavedSpeciesForGarden(+userId, +gardenId);
    }
    return this.speciesService.getSavedSpecies(+userId);
  }

  @Post('save/:trefleId')
  saveSpecies(
    @Param('trefleId') trefleId: string,
    @Query('userId') userId: string,
    @Query('gardenId') gardenId?: string,
  ) {
    if (gardenId) {
      return this.speciesService.saveSpeciesToGarden(+userId, +trefleId, +gardenId);
    }
    return this.speciesService.saveSpecies(+userId, +trefleId);
  }

  @Delete('save/:trefleId')
  unsaveSpecies(
    @Param('trefleId') trefleId: string,
    @Query('userId') userId: string,
    @Query('gardenId') gardenId?: string,
  ) {
    if (gardenId) {
      return this.speciesService.unsaveSpeciesFromGarden(+userId, +trefleId, +gardenId);
    }
    return this.speciesService.unsaveSpecies(+userId, +trefleId);
  }

  @Post()
  create(@Body() createSpeciesDto: CreateSpeciesDto) {
    return this.speciesService.create(createSpeciesDto);
  }

  @Get()
  findAll() {
    return this.speciesService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.speciesService.findOne(+id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateSpeciesDto: UpdateSpeciesDto) {
    return this.speciesService.update(+id, updateSpeciesDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.speciesService.remove(+id);
  }
}
