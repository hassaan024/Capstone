import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { OPEN_METEO_CURRENT_FORCAST_URL } from 'utils/constants';
import { WeatherInfoDto } from './dto/weather-info.dto';
import { celsiusToFahrenheit, metersPerSecondToMph } from 'utils/util-functions';
import { ServerDescription } from 'typeorm';
import { hourlyToDaily } from 'utils/array';

interface OpenMeteoCurrent {
  temperature_2m: number;
  relative_humidity_2m: number;
  wind_speed_10m: number;
  weather_code: number;
  shortwave_radiation: number;
  precipitation: number;
  elevation: number;
}

interface OpenMeteoHourly {
  vapour_pressure_deficit?: number[];
  soil_temperature_6cm?: number[];
  soil_moisture_0_to_3cm?: number[];
  wind_speeds_10m?: number[];
  relative_humidity_2m?: number[];
}

interface OpenMeteoDaily {
  et0_fao_evapotranspiration?: number[];
  precipitation_sum?: number[];
  shortwave_radiation_sum?: number[];
  temperature_2m_max?: number[];
  temperature_2m_min?: number[];
}


interface OpenMeteoResponse {
  current?: OpenMeteoCurrent;
  hourly?: OpenMeteoHourly;
  daily?: OpenMeteoDaily;
  timezone: string;
  utc_offset_seconds: number;
}

const current_args: string[] = [
  'temperature_2m', // temp 2m above the ground
  'relative_humidity_2m', // humidity 2m above the ground relative to the temperature
  'wind_speed_10m', // wind speed 10m above the ground
  'weather_code', // numeric code representing the current weather at given location
  'shortwave_radiation', // sun intensity (W/m² at this moment)
  'precipitation', // all forms of water falling from the sky (current)
];

const hourly_args: string[] = [
  'vapour_pressure_deficit', // How strongly the air is pulling water out of the plant right now (varies hourly)
];

// FIXME: 'soil_temperature_6cm', // temp of soil 6cm under ground
// FIXME: 'soil_moisture_0_to_3cm', // moisture between 0-3 cm (volumetric water content)

const daily_args: string[] = [
  // If your plants were well-watered, this is how much water the weather 
  // would make them lose today through evaporation from the soil and water leaving the leaves (mm/day)
  'et0_fao_evapotranspiration',
  'precipitation_sum', // total precipitation for the day (mm)
  'shortwave_radiation_sum',  // total sunlight energy received during the day
];

type WeatherDescription =
  typeof weatherCodeMap[keyof typeof weatherCodeMap] | 'Unknown';

const weatherCodeMap = {
  0: 'Clear sky',
  1: 'Partly cloudy',
  2: 'Partly cloudy',
  3: 'Overcast',
  45: 'Fog',
  48: 'Fog',
  51: 'Rain',
  61: 'Rain',
  71: 'Snow',
} as const;

@Injectable()
export class WeatherService {
  private readonly logger = new Logger(WeatherService.name);

  // GET (current days weather) ##################################################################
  // FIXME: add soil_temperature_6cm
  // FIXME: add soil_moisture_0_to_3cm
  async getCurrentDaysWeather(
    latitude: number,
    longitude: number,
  ): Promise<WeatherInfoDto> {

    const url =
    `${OPEN_METEO_CURRENT_FORCAST_URL}?latitude=${latitude}` +
    `&longitude=${longitude}` +
    `&current=${current_args.join(',')}` +
    `&hourly=${hourly_args.join(',')}` +
    `&daily=${daily_args.join(',')}` +
    `&models=ecmwf_ifs025` +
    `&timezone=auto`;

    this.logger.log(`URL: ${url}`)
    this.logger.log(`Fetching current weather for ${latitude}, ${longitude}`);

    const res = await fetch(url);

    if (!res.ok) {
      const errorText = await res.text(); // or res.json()

      this.logger.error(`
        ===============================================
        Open-Meteo API Error
        Status: ${res.status}
        StatusText: ${res.statusText}
        Body: ${errorText}
        URL: ${url}
        ===============================================
      `);

      throw new BadRequestException(
        `Weather API error (${res.status}): ${errorText}`
      );
    }

    const data = (await res.json()) as OpenMeteoResponse;

    this.logger.log(`
      ===============================================
      ${JSON.stringify(data, null, 2)}
      ===============================================
    `);

    if (!data.current) {
      throw new BadRequestException('Weather data unavailable');
    }
    
    const current = data.current;
    const hourly = data.hourly;
    const daily = data.daily;
    const description: WeatherDescription = weatherCodeMap[current.weather_code] ?? 'Unknown';

    const current_weather: WeatherInfoDto = {
      elevation: current.elevation,
      temperature_2m: celsiusToFahrenheit(current.temperature_2m),
      relative_humidity_2m: current.relative_humidity_2m,
      wind_speed_10m: metersPerSecondToMph(current.wind_speed_10m),
      sunlight_intensity: daily.shortwave_radiation_sum[0] ?? 0,
      precipitation: daily.precipitation_sum[0] ?? 0,

      // Daily → take first element (today)
      daily_evaporation: daily?.et0_fao_evapotranspiration?.[0] ?? 0,

      // Hourly → take first element (current hour)
      vapour_pressure_deficit: hourly?.vapour_pressure_deficit?.[0] ?? 0,
      soil_temperature_6cm: hourly?.soil_temperature_6cm?.[0] ?? 0,
      soil_moisture_0_to_3cm: hourly?.soil_moisture_0_to_3cm?.[0] ?? 0,

      timezone: data.timezone,
      utc_offset_seconds: data.utc_offset_seconds,
      description: description
    };

    return current_weather;
  }

  // GET (current weeks weather) ##########################################################################################
  // FIXME: add soil_temperature_6cm
  async getWeeklyForecast(latitude: number, longitude: number): Promise<WeatherInfoDto[]> {
    const dailyArgs = [
      'temperature_2m_max', 
      'temperature_2m_min',
      'precipitation_sum',
      'et0_fao_evapotranspiration',
      'shortwave_radiation_sum',
    ];

    const hourlyArgs = ['vapour_pressure_deficit', 'wind_speed_10m', 'soil_temperature_6cm', 'relative_humidity_2m'];

    const url = `${OPEN_METEO_CURRENT_FORCAST_URL}?latitude=${latitude}&longitude=${longitude}` +
      `&daily=${dailyArgs.join(',')}` +
      `&hourly=${hourlyArgs.join(',')}` +
      `&timezone=auto`;

    const res = await fetch(url);
    
    if (!res.ok) {
      const errorText = await res.text();

      this.logger.error(`
        ===============================================
        Open-Meteo API Error
        Status: ${res.status}
        StatusText: ${res.statusText}
        Body: ${errorText}
        URL: ${url}
        ===============================================
      `);

      throw new BadRequestException(
        `Weather API error (${res.status}): ${errorText}`
      );
    }

    const data = await res.json();
    
    // Direclty on data
    const utc_offset_seconds: number = data.utc_offset_seconds ?? 0;
    const timezone: string = data.timezone ?? "Unknown";
    const elevation: number = data.elevation ?? 0;
    
    // Hourly
    const hourly = data.hourly;

    // Note: I convert each to a weekly array
    const hourly_info: OpenMeteoHourly = {
      vapour_pressure_deficit: (
        hourlyToDaily(hourly.vapour_pressure_deficit ?? [])
      ),
      wind_speeds_10m: (
        hourlyToDaily(hourly.wind_speed_10m ?? [])
      ),
      soil_temperature_6cm: (
        hourlyToDaily(hourly.soil_temperature_6cm ?? [])
      ),
      relative_humidity_2m: (
        hourlyToDaily(hourly.relative_humidity_2m ?? [])
      )
    }

    // Daily
    const daily = data.daily;
    // Safely get daily arrays or empty arrays
    const daily_info: OpenMeteoDaily = {
      temperature_2m_max: daily?.temperature_2m_max ?? [],
      temperature_2m_min: daily?.temperature_2m_min ?? [],
      precipitation_sum: daily?.precipitation_sum ?? [],
      et0_fao_evapotranspiration: daily?.et0_fao_evapotranspiration ?? [],
      shortwave_radiation_sum: daily?.shortwave_radiation_sum ?? []
    }

    const forecast: WeatherInfoDto[] = [];
    const n: number = data.daily.time.length;
    
    for (let i = 0; i < n; i += 1) {
      const avg_temp: number = ((daily_info.temperature_2m_max[i] ?? 0) + (daily_info.temperature_2m_min[i] ?? 0)) / 2;
      const weather: WeatherInfoDto = {
        elevation: elevation,
        temperature_2m: celsiusToFahrenheit(avg_temp),
        relative_humidity_2m: hourly_info.relative_humidity_2m[i] ?? 0,
        wind_speed_10m: hourly_info.wind_speeds_10m[i] ?? 0,
        sunlight_intensity: daily_info.shortwave_radiation_sum[i] ?? 0,
        precipitation: daily_info.precipitation_sum[i] ?? 0,
        daily_evaporation: daily_info.et0_fao_evapotranspiration[i] ?? 0,
        vapour_pressure_deficit: hourly_info.vapour_pressure_deficit[i] ?? 0,
        soil_temperature_6cm: celsiusToFahrenheit(hourly_info.soil_temperature_6cm[i] ?? 0),
        soil_moisture_0_to_3cm: 0,
        timezone: timezone,
        utc_offset_seconds: utc_offset_seconds,
      }
      forecast.push(weather);
    }
    return forecast;
  }

  // GET (current historical weather)
  // FIXME: Just implement this
  async getPastForecast(
    latitude: number, longitude: number, days: number, offset: number
  ): Promise<WeatherInfoDto[]> {
    this.logger.log(`
      Getting the forcast from the past ${days} days on page ${offset}.
      At Long ${latitude}, Lat ${longitude}.
    `)
    return [];
  }

}


