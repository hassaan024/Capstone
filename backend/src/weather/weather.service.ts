import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { OPEN_METEO_CURRENT_FORCAST_URL } from 'utils/constants';
import { CurrentWeatherDto } from './dto/current-weather.dto';
import { celsiusToFahrenheit, metersPerSecondToMph } from 'utils/util-functions';

interface OpenMeteoCurrent {
  temperature_2m: number;
  relative_humidity_2m: number;
  wind_speed_10m: number;
  weather_code: number;
}

interface OpenMeteoResponse {
  current?: OpenMeteoCurrent;
}

@Injectable()
export class WeatherService {
  private readonly logger = new Logger(WeatherService.name);

  async getCurrentWeather(
    latitude: number,
    longitude: number,
  ): Promise<CurrentWeatherDto> {
    const url =
      `${OPEN_METEO_CURRENT_FORCAST_URL}` +
      `?latitude=${latitude}` +
      `&longitude=${longitude}` +
      `&current=temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code` +
      `&timezone=auto`;

    this.logger.log(`Fetching weather for ${latitude}, ${longitude}`);

    const res = await fetch(url);

    if (!res.ok) {
      throw new BadRequestException('Failed to fetch weather data');
    }

    const data = (await res.json()) as OpenMeteoResponse;

    if (!data.current) {
      throw new BadRequestException('Weather data unavailable');
    }

    const current = data.current;
    const { description, icon } = this.mapWeatherCode(current.weather_code);

    const current_weather: CurrentWeatherDto = {
      temperature: celsiusToFahrenheit(current.temperature_2m),
      humidity: current.relative_humidity_2m,
      windSpeed: metersPerSecondToMph(current.wind_speed_10m),
      description,
    };

    return current_weather;
  }

  private mapWeatherCode(code: number) {
    //  these codes are defined in the api
    switch (code) {
      case 0:
        return { description: 'Clear sky', icon: '/icons/sun.svg' };
      case 1:
      case 2:
        return { description: 'Partly cloudy', icon: '/icons/cloud-sun.svg' };
      case 3:
        return { description: 'Overcast', icon: '/icons/cloud.svg' };
      case 45:
      case 48:
        return { description: 'Fog', icon: '/icons/fog.svg' };
      case 51:
      case 61:
        return { description: 'Rain', icon: '/icons/rain.svg' };
      case 71:
        return { description: 'Snow', icon: '/icons/snow.svg' };
      default:
        return { description: 'Unknown', icon: '/icons/cloud.svg' };
    }
  }
}
