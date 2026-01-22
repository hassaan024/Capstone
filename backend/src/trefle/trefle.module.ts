import { HttpModule } from '@nestjs/axios';
import { Module } from '@nestjs/common';
import { TrefleController } from './trefle.controller.js';
import { TrefleService } from './trefle.service.js';
import { DatabaseModule } from 'database/database.module';

@Module({
  imports: [HttpModule, DatabaseModule],
  controllers: [TrefleController],
  providers: [TrefleService],
  exports: [TrefleService],
})
export class TrefleModule {}
