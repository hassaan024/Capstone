import { Controller, Get, Param, Query, Logger, BadRequestException } from '@nestjs/common';
import { TrefleService } from './trefle.service.js';

@Controller('trefle')
export class TrefleController {
  private readonly logger = new Logger(TrefleController.name);

  constructor(private readonly trefleService: TrefleService) {}

  @Get('search')
  async search(@Query('q') q: string) {
    if (!q) {
      throw new BadRequestException('Query parameter "q" is required');
    }
    this.logger.log(`Searching plants in Trefle for query: ${q}`);
    return this.trefleService.search(q);
  }

  @Get('species/:id')
  async getDetails(@Param('id') id: string) {
    this.logger.log(`Fetching detailed info for Trefle species ID: ${id}`);
    const numericId = parseInt(id, 10);
    if (isNaN(numericId)) {
      throw new BadRequestException('Invalid ID');
    }
    return this.trefleService.getDetails(numericId);
  }
}
