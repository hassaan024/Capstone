import { Controller } from '@nestjs/common';
import { DatabaseService } from './database.service';

@Controller()
export class AppController {
  constructor(private readonly databaseService: DatabaseService) {}
}
