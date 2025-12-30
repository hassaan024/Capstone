import { Module } from '@nestjs/common';
import { SoilService } from './soil.service.js';
import { SoilController } from './soil.controller.js';

@Module({
  controllers: [SoilController],
  providers: [SoilService],
})
export class SoilModule {}
