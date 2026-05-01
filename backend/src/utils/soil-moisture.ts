import { WeatherInfoDto } from 'weather/dto/weather-info.dto';

const RETENTION: Record<string, number> = {
  CLAY:  0.8,
  LOAM:  0.6,
  SILT:  0.6,
  PEAT:  0.7,
  SANDY: 0.3,
  CHALK: 0.4,
};

export function estimateSoilMoisture(
  weather: WeatherInfoDto[],
  soilType: string,
): number {
  const retention = RETENTION[soilType] ?? 0.5;
  let moisture = 0.5;
  let total = 0;

  for (const day of weather) {
    moisture += day.precipitation * 0.04;
    moisture -= day.daily_evaporation * (1 - retention) * 0.03;
    moisture = Math.min(Math.max(moisture, 0), 1);
    total += moisture;
  }

  const avg = weather.length > 0 ? total / weather.length : moisture;
  return Math.round(avg * 100) / 100;
}