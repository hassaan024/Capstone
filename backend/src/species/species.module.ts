import { Module } from '@nestjs/common';
import { SpeciesService } from './species.service.js';
import { SpeciesController } from './species.controller.js';
import { TrefleModule } from '../trefle/trefle.module.js';
import { PerenualModule } from '../perenual/perenual.module.js';

@Module({
  imports: [TrefleModule, PerenualModule],
  controllers: [SpeciesController],
  providers: [SpeciesService],
})
export class SpeciesModule {}
