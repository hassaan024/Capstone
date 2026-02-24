import { Injectable, Logger, BadRequestException } from '@nestjs/common';
import { OPEN_METEO_ARCHIVE_FORCAST_URL, OPEN_METEO_CURRENT_FORCAST_URL } from 'utils/constants';
import { WeatherInfoDto } from './dto/weather-info.dto';
import { celsiusToFahrenheit, metersPerSecondToMph } from 'utils/util-functions';
import { hourlyToDaily } from 'utils/array';
import { 
  getTodaysDateAsString, 
  calculatePastDate,
  goForwardDays,
  goBackDays,
} from 'utils/time';
import { PAGE_SIZE } from './weather_api_constants';

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

    const start_date = getTodaysDateAsString();
    const end_date = goForwardDays(start_date, 6);

    const url = `${OPEN_METEO_CURRENT_FORCAST_URL}?latitude=${latitude}&longitude=${longitude}` +
      `&daily=${dailyArgs.join(',')}` +
      `&hourly=${hourlyArgs.join(',')}` +
      `&start_date=${start_date}&end_date=${end_date}` +
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
    return this.mapWeatherApiDataToDto(data, false);
  }

  // GET (current historical weather)
  // FIXME: Just implement this
  async getPastForecast(
    latitude: number, longitude: number, days: number, offset: number
  ): Promise<any> {

    this.logger.log(`
      Getting the forcast from the past ${days} days on page ${offset}.
      At Long ${latitude}, Lat ${longitude}.
    `)
    
    // cacluate the range of days we are fetching
    let start_date: string;
    let end_date: string;

    if (offset === 0){
      start_date = goBackDays(getTodaysDateAsString(), days);
      end_date = getTodaysDateAsString();
    } 
    else {
      start_date = calculatePastDate(offset, PAGE_SIZE); // page start
      end_date = goForwardDays(start_date, days); // page end
    }

    this.logger.log(`
      Start: ${start_date}
      End: ${end_date}
    `)

    const daily_args = [
      'temperature_2m_max', 
      'temperature_2m_min',
      'precipitation_sum',
      'et0_fao_evapotranspiration',
      'shortwave_radiation_sum',
    ];

    const hourly_args = [
      'vapour_pressure_deficit', 
      'wind_speed_10m',                   
      'relative_humidity_2m'
    ];
    
    const url = `${OPEN_METEO_ARCHIVE_FORCAST_URL}?latitude=${latitude}&longitude=${longitude}` +
      `&start_date=${start_date}&end_date=${end_date}` +
      `&daily=${daily_args.join(',')}` +
      `&hourly=${hourly_args.join(',')}` +
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
      const data = await res.json();
      throw new BadRequestException(
        `Weather API error (${res.status}): ${errorText}`
      );
    }
    const data = await res.json();
    return this.mapWeatherApiDataToDto(data, true);
  }

  private mapWeatherApiDataToDto(data: any, past: boolean): WeatherInfoDto[] {
    const timezone = data.timezone ?? 'Unknown';
    const utc_offset_seconds = data.utc_offset_seconds ?? 0;
    const elevation = data.elevation ?? 0;

    const n = data.daily.time.length;
    const forecast: WeatherInfoDto[] = [];

    for (let i = 0; i < n; i++) {
      const max = data.daily.temperature_2m_max?.[i] ?? 0;
      const min = data.daily.temperature_2m_min?.[i] ?? 0;
      const avgTemp = (max + min) / 2;
      
      const weather: WeatherInfoDto = {
        elevation,
        temperature_2m: Math.round(celsiusToFahrenheit(avgTemp) * 100) / 100,
        relative_humidity_2m: hourlyToDaily(data.hourly.relative_humidity_2m ?? [])[i] ?? 0,
        wind_speed_10m: metersPerSecondToMph(hourlyToDaily(data.hourly.wind_speed_10m ?? [])[i] ?? 0),
        sunlight_intensity: data.daily.shortwave_radiation_sum?.[i] ?? 0,
        precipitation: data.daily.precipitation_sum?.[i] ?? 0,
        daily_evaporation: data.daily.et0_fao_evapotranspiration?.[i] ?? 0,
        vapour_pressure_deficit: hourlyToDaily(data.hourly.vapour_pressure_deficit ?? [])[i] ?? 0,
        soil_moisture_0_to_3cm: hourlyToDaily(data.hourly.soil_moisture_0_to_3cm ?? [])[i] ?? 0,
        timezone,
        utc_offset_seconds,
      }
      
      if (!past) {
        weather.soil_temperature_6cm =  Math.round(celsiusToFahrenheit(hourlyToDaily(data.hourly.soil_temperature_6cm ?? [])[i] ?? 0) * 100) / 100
      }
      forecast.push(weather);
    }

    return forecast;
  }
}

