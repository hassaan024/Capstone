import { Injectable, Logger } from '@nestjs/common';
import { DatabaseService } from 'database/database.service';
import { SearchPlantsQueryDto } from './dto/search-plants-query.dto';

const BASE_PERENUAL_URL: string = 'https://perenual.com/api/v2/'

@Injectable()
export class PerenualService {
    private API_KEY = process.env.PERENUAL_API_TOKEN;;
    constructor(private readonly db: DatabaseService) {}
    private readonly logger = new Logger(PerenualService.name);
    
    // seach plants using a search string
    async searchPlants(queryDto: SearchPlantsQueryDto) {
        const url = `${BASE_PERENUAL_URL}species-list?key=${this.API_KEY}&q=${queryDto.query}`;
        const response = await fetch(url);
        const data = await response.json();
        return data;
    }

    async getPlantDetails(id: number) {
        this.logger.log("HITTING")
        const url = `${BASE_PERENUAL_URL}species/details/${id}?key=${this.API_KEY}`;
        const response = await fetch(url);
        const data = await response.json();
        return data;
    }
    
}
