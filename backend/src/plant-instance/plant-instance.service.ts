import { Injectable } from '@nestjs/common';
import { CreatePlantInstanceDto } from './dto/create-plant-instance.dto.js';
import { UpdatePlantInstanceDto } from './dto/update-plant-instance.dto.js';

@Injectable()
export class PlantInstanceService {
  create(createPlantInstanceDto: CreatePlantInstanceDto) {
    return 'This action adds a new plantInstance';
  }

  findAll() {
    return `This action returns all plantInstance`;
  }

  findOne(id: number) {
    return `This action returns a #${id} plantInstance`;
  }

  update(id: number, updatePlantInstanceDto: UpdatePlantInstanceDto) {
    return `This action updates a #${id} plantInstance`;
  }

  remove(id: number) {
    return `This action removes a #${id} plantInstance`;
  }
}
