import { Module } from '@nestjs/common';
import { SoilService } from './soil.service.js';

@Module({
  controllers: [],
  providers: [SoilService],
})
export class SoilModule {}
