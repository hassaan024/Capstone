import { Module } from '@nestjs/common';
import { SpeciesService } from './species.service';
import { SpeciesController } from './species.controller';
import { TrefleModule } from '../trefle/trefle.module';
import { PerenualModule } from '../perenual/perenual.module';

@Module({
  imports: [TrefleModule, PerenualModule],
  controllers: [SpeciesController],
  providers: [SpeciesService],
})
export class SpeciesModule {}
