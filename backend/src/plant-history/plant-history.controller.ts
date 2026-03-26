import { Controller, Get, Post, Delete, Param, Body, Query } from '@nestjs/common';
import { PlantHistoryService } from './plant-history.service';
import { CreatePlantHistoryDto } from './dto/create-plant-history.dto';

@Controller('plant-history')
export class PlantHistoryController {
  constructor(private readonly historyService: PlantHistoryService) {}

  // Add a new history record
  @Post()
  async create(@Body() createDto: CreatePlantHistoryDto) {
    return this.historyService.addRecord(createDto);
  }

  // Get all history for a plant, optionally filter by start/end dates
  @Get(':plantId')
  async getHistory(
    @Param('plantId') plantId: string,
    @Query('start') start?: string,
    @Query('end') end?: string,
  ) {
    const startDate = start ? new Date(start) : undefined;
    const endDate = end ? new Date(end) : undefined;
    return this.historyService.getHistory(+plantId, startDate, endDate);
  }

  // 3️⃣ Get summary for a plant
  @Get(':plantId/summary')
  async getSummary(@Param('plantId') plantId: string) {
    return this.historyService.getSummary(+plantId);
  }

  // Get the latest record for a plant
  @Get(':plantId/latest')
  async getLatest(@Param('plantId') plantId: string) {
    return this.historyService.getLatest(+plantId);
  }
}