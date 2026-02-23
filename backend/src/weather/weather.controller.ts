import { Controller, Get, ParseIntPipe, Query } from '@nestjs/common';
import { WeatherService } from './weather.service';
import { WeatherInfoDto } from './dto/weather-info.dto';
import { ValidateLocationPipe } from './pipes/location-validation-pipe';

const PAGE_SIZE = 100;

@Controller('weather')
export class WeatherController {
  constructor(private readonly weatherService: WeatherService) {}

  // Current weather
  @Get('/current')
  async getCurrentDaysWeather(
    @Query('latitude', ValidateLocationPipe) latitude: number,
    @Query('longitude', ValidateLocationPipe) longitude: number,
  ): Promise<WeatherInfoDto> {
    return await this.weatherService.getCurrentDaysWeather(latitude, longitude);
  }

  // get forcast for the next 7 days
  @Get('/future')
  async getWeeklyForecast(
    @Query('latitude', ValidateLocationPipe) latitude: number,
    @Query('longitude', ValidateLocationPipe) longitude: number,
  ): Promise<WeatherInfoDto[]> {
    return await this.weatherService.getWeeklyForecast(latitude, longitude);
  }

  /*
  get forcast from the past x days uses a pageing  to implement fetches
  page size = 100
  */
  @Get('/past')
  async getPastForecast(
    @Query('latitude', ValidateLocationPipe) latitude: number,
    @Query('longitude', ValidateLocationPipe) longitude: number,
    @Query('days') days?: string,  // optional
    @Query('page') page?: string,  // optional
  ): Promise<WeatherInfoDto[]> {
    const days_num = (days !== undefined) ? parseInt(days, 10) : 7;
    const page_num = (page !== undefined) ? parseInt(page, 10) : 1;
    const days_capped = Math.min(days_num, PAGE_SIZE);
    // // calculate offset for paging
    const offset = (page_num - 1) * PAGE_SIZE;
    return await this.weatherService.getPastForecast(latitude, longitude, days_capped, offset);
  }
}