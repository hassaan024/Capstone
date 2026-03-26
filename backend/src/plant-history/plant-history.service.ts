import { Injectable } from '@nestjs/common';
import { Prisma } from '@prisma/client';
import { CreatePlantHistoryDto } from './dto/create-plant-history.dto';
import { DatabaseService } from 'database/database.service';

@Injectable()
export class PlantHistoryService {
  constructor(private readonly db: DatabaseService) {}

  /**
   * Add a new history record for a plant
   */
  async addRecord(createDto: CreatePlantHistoryDto) {
    const { plantId, date, heightCm, predictedValue } = createDto;

    return this.db.plantHistory.create({
      data: {
        plantId,
        date: new Date(date),
        heightCm,
        predictedValue,
      },
    });
  }

  /**
   * Get all history records for a plant, optionally filtered by start/end dates
   */
  async getHistory(plantId: number, start?: Date, end?: Date) {
    const where: any = { plantId };

    if (start || end) {
      where.date = {};
      if (start) where.date.gte = start;
      if (end) where.date.lte = end;
    }

    return this.db.plantHistory.findMany({
      where,
      orderBy: { date: 'asc' },
    });
  }

  /**
   * Get aggregated summary for a plant
   * Example: average height, min/max, record count
   */
  async getSummary(plantId: number) {
    const aggregation = await this.db.plantHistory.aggregate({
      where: { plantId },
      _avg: { heightCm: true, predictedValue: true },
      _min: { heightCm: true },
      _max: { heightCm: true },
      _count: { id: true },
    });

    return {
      plantId,
      records: aggregation._count.id,
      avgHeightCm: aggregation._avg.heightCm,
      minHeightCm: aggregation._min.heightCm,
      maxHeightCm: aggregation._max.heightCm,
      avgPredictedValue: aggregation._avg.predictedValue,
    };
  }

  /**
   * Get the most recent history record for a plant
   */
  async getLatest(plantId: number) {
    return this.db.plantHistory.findFirst({
      where: { plantId },
      orderBy: { date: 'desc' },
    });
  }
}