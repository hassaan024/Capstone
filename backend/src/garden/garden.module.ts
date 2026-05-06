import { Module, forwardRef } from '@nestjs/common';
import { GardenService } from './garden.service';
import { GardenController } from './garden.controller';
import { DatabaseModule } from 'database/database.module';
import { EmailModule } from '../email/email.module';
import { GardenScheduler } from './garden.scheduler';

@Module({
  imports: [DatabaseModule, EmailModule],
  controllers: [GardenController],
  providers: [GardenService, GardenScheduler],
  exports: [GardenScheduler],
})
export class GardenModule {}
