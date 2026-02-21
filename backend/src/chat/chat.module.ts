import { Module } from '@nestjs/common';
import { ChatController } from './chat.controller.js';
import { ChatService } from './chat.service.js';
import { DatabaseModule } from 'database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [ChatController],
  providers: [ChatService],
})
export class ChatModule {}
