import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { PlantInstanceService } from './plant-instance.service.js';
import { CreatePlantInstanceDto } from './dto/create-plant-instance.dto.js';
import { UpdatePlantInstanceDto } from './dto/update-plant-instance.dto.js';

@Controller('plant-instance')
export class PlantInstanceController {
  constructor(private readonly plantInstanceService: PlantInstanceService) {}

  @Post()
  create(@Body() createPlantInstanceDto: CreatePlantInstanceDto) {
    return this.plantInstanceService.create(createPlantInstanceDto);
  }

  @Get()
  findAll() {
    return this.plantInstanceService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.plantInstanceService.findOne(+id);
  }

  @Patch(':id')
  update(
    @Param('id') id: string,
    @Body() updatePlantInstanceDto: UpdatePlantInstanceDto,
  ) {
    return this.plantInstanceService.update(+id, updatePlantInstanceDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.plantInstanceService.remove(+id);
  }
}
