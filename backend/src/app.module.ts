import { Module } from '@nestjs/common';
import { AppController } from './app.controller.js';
import { AppService } from './app.service.js';
import { UserModule } from './user/user.module.js';
import { GardenModule } from './garden/garden.module.js';
import { PlantInstanceModule } from './plant-instance/plant-instance.module.js';
import { SpeciesModule } from './species/species.module.js';
import { SoilModule } from './soil/soil.module.js';
import { DatabaseModule } from './database/database.module.js';

@Module({
  imports: [
    DatabaseModule,
    UserModule,
    GardenModule,
    PlantInstanceModule,
    SpeciesModule,
    SoilModule,
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
