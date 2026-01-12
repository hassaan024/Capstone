import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { UserModule } from './user/user.module';
import { GardenModule } from './garden/garden.module';
import { PlantInstanceModule } from './plant-instance/plant-instance.module';
import { SpeciesModule } from './species/species.module';
import { SoilModule } from './soil/soil.module';
import { DatabaseModule } from './database/database.module';
import { AuthModule } from './auth/auth.module';

@Module({
  imports: [
    DatabaseModule,
    AuthModule,
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
