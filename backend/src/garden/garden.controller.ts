import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { GardenService } from './garden.service.js';
import { CreateGardenDto } from './dto/create-garden.dto.js';
import { UpdateGardenDto } from './dto/update-garden.dto.js';

@Controller('garden')
export class GardenController {
  constructor(private readonly gardenService: GardenService) {}

  @Post()
  create(@Body() createGardenDto: CreateGardenDto) {
    return this.gardenService.create(createGardenDto);
  }

  @Get()
  findAll() {
    return this.gardenService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.gardenService.findOne(+id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateGardenDto: UpdateGardenDto) {
    return this.gardenService.update(+id, updateGardenDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.gardenService.remove(+id);
  }
}
