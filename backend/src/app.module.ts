import { Module } from '@nestjs/common';
import { AppController } from './app.controller.js';
import { AppService } from './app.service.js';
import { DatabaseModule } from 'database/database.module';
import { UserModule } from './user/user.module.js';
import { GardenModule } from './garden/garden.module.js';
import { SoilModule } from './soil/soil.module.js';
import { AuthModule } from './auth/auth.module.js';
import { TrefleModule } from './trefle/trefle.module.js';
import { SpeciesModule } from './species/species.module.js';
import { PlantShoppingModule } from './plant-shopping/plant-shopping.module.js';
import { ConfigModule } from '@nestjs/config';

@Module({
  imports: [
    ConfigModule.forRoot({ isGlobal: true }),
    DatabaseModule,
    UserModule,
    GardenModule,
    SoilModule,
    AuthModule,
    TrefleModule,
    SpeciesModule,
    PlantShoppingModule,
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
