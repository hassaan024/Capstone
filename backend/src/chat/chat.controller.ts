import {
  Controller,
  Post,
  Body,
  Logger,
  BadRequestException,
} from '@nestjs/common';
import { ChatService } from './chat.service.js';

interface ChatMessageDto {
  message: string;
  userId?: number;
  history?: { role: 'user' | 'model'; parts: { text: string }[] }[];
}

@Controller('chat')
export class ChatController {
  private readonly logger = new Logger(ChatController.name);

  constructor(private readonly chatService: ChatService) {}

  @Post('message')
  async sendMessage(@Body() body: ChatMessageDto) {
    if (!body.message || !body.message.trim()) {
      throw new BadRequestException('Message is required');
    }

    this.logger.log(
      `Chat message from user ${body.userId || 'anonymous'}: "${body.message.substring(0, 50)}..."`,
    );

    return this.chatService.sendMessage(
      body.message,
      body.userId,
      body.history,
    );
  }
}
