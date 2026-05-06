import { Module } from '@nestjs/common';
import { PlantInstanceService } from './plant-instance.service';
import { PlantInstanceController } from './plant-instance.controller';
import { GardenModule } from '../garden/garden.module';

@Module({
  imports: [GardenModule],
  controllers: [PlantInstanceController],
  providers: [PlantInstanceService],
})
export class PlantInstanceModule {}
