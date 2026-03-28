import { Module } from '@nestjs/common';
import { PerenualService } from './perenual.service';
import { PerenualController } from './perenual.controller';

@Module({
  providers: [PerenualService],
  controllers: [PerenualController],
  exports: [PerenualService],
})
export class PerenualModule {}
