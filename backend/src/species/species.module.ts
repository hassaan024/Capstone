import { Module } from '@nestjs/common';
import { SpeciesService } from './species.service.js';
import { SpeciesController } from './species.controller.js';

@Module({
  controllers: [SpeciesController],
  providers: [SpeciesService],
})
export class SpeciesModule {}
