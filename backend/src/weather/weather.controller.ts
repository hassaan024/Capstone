import { Controller, Get, Query } from '@nestjs/common';
import { ValidateLocationPipe } from './pipes/location-validation-pipe';
import { WeatherService } from './weather.service';
import { CurrentWeatherDto } from './dto/current-weather.dto';

@Controller('weather')
export class WeatherController {
  constructor(private readonly weatherService: WeatherService) {}

  @Get('/current')
  async getCurrentWeather(
    @Query('latitude', ValidateLocationPipe) latitude: number,
    @Query('longitude', ValidateLocationPipe) longitude: number,
  ): Promise<CurrentWeatherDto> {
    return await this.weatherService.getCurrentWeather(latitude, longitude);
  }
}
