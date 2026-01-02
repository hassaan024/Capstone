import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module.js';
import 'dotenv/config';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  app.enableCors();
  app.setGlobalPrefix('backend');
  console.log(`Listening on port ${process.env.BACKEND_PORT ?? 3000}`);
  await app.listen(process.env.BACKEND_PORT ?? 3000);
}

void bootstrap();
