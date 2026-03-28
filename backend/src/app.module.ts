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
import { ChatModule } from './chat/chat.module.js';
import { ConfigModule } from '@nestjs/config';
import { WeatherModule } from './weather/weather.module';
import { PredictionModule } from './prediction/prediction.module';
import { PlantHistoryModule } from './plant-history/plant-history.module';
import { PerenualModule } from './perenual/perenual.module';

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
    WeatherModule,
    PlantShoppingModule,
    ChatModule,
    PredictionModule,
    PlantHistoryModule,
    PerenualModule,
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
