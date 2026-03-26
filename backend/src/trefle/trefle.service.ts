import { HttpService } from '@nestjs/axios';
import { Injectable, Logger, HttpException, HttpStatus } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';
import { firstValueFrom } from 'rxjs';
import { DatabaseService } from 'database/database.service';

@Injectable()
export class TrefleService {
  private readonly logger = new Logger(TrefleService.name);
  private readonly apiToken: string;
  private readonly baseUrl = 'https://trefle.io/api/v1';

  constructor(
    private readonly httpService: HttpService,
    private readonly configService: ConfigService,
    private readonly db: DatabaseService,
  ) {
    this.apiToken = this.configService.get<string>('TREFLE_API_TOKEN') || process.env.TREFLE_API_TOKEN || '';
    
    if (!this.apiToken) {
      this.logger.warn('TREFLE_API_TOKEN is not set! External plant data fetching will fail.');
    }
  }

  async search(query: string) {
    if (!this.apiToken) {
      throw new HttpException('API Token configuration missing', HttpStatus.INTERNAL_SERVER_ERROR);
    }

    try {
      const url = `${this.baseUrl}/plants/search?token=${this.apiToken}&q=${encodeURIComponent(query)}`;
      const { data } = await firstValueFrom(this.httpService.get(url));
      return data;
    } catch (error) {
      this.logger.error(`Error searching plants in Trefle: ${(error as Error).message}`);
      throw new HttpException('Failed to fetch from Trefle API', HttpStatus.BAD_GATEWAY);
    }
  }

  async getDetails(id: number) {
    if (!this.apiToken) {
      throw new HttpException('API Token configuration missing', HttpStatus.INTERNAL_SERVER_ERROR);
    }

    try {
      // Check if we already have this species detailed info cached/saved?
      // For now, just fetch from API
      
      const url = `${this.baseUrl}/plants/${id}?token=${this.apiToken}`;
      const { data } = await firstValueFrom(this.httpService.get(url));
      
      return data;
    } catch (error) {
      this.logger.error(`Error fetching plant details from Trefle: ${(error as Error).message}`);
      throw new HttpException('Failed to fetch plant details from Trefle', HttpStatus.BAD_GATEWAY);
    }
  }
}
