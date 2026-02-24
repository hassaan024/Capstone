import { Controller, Get, ParseIntPipe, Query } from '@nestjs/common';
import { WeatherService } from './weather.service';
import { WeatherInfoDto } from './dto/weather-info.dto';
import { ValidateLocationPipe } from './pipes/location-validation-pipe';
import { PAGE_SIZE } from './weather_api_constants';

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
    @Query('offset') offset?: string,  // optional
  ): Promise<any> {
    let days_num = (days !== undefined) ? parseInt(days, 10) : 7;
    let offset_num: number = (offset !== undefined) ? parseInt(offset, 10) : 0;
    days_num = Math.min(days_num, PAGE_SIZE);
    // // calculate offset for paging
    offset_num = (offset_num < 0)? 0 : offset_num;
    days_num = (days_num < 0)? 0 : days_num;
    if (days_num == 0) {
      return []
    }
    return await this.weatherService.getPastForecast(latitude, longitude, days_num - 1, offset_num);
  }
}