import { PartialType } from '@nestjs/mapped-types';
import { CreatePlantInstanceDto } from './create-plant-instance.dto';

export class UpdatePlantInstanceDto extends PartialType(
  CreatePlantInstanceDto,
) {}
