import React, { useEffect, useState } from "react";
import { BACKEND_BASE_URL } from "../utils/constants";
import {
  WiDaySunny,
  WiCloudy,
  WiCloud,
  WiFog,
  WiRain,
  WiSnow,
  WiBarometer,
  WiHumidity,
  WiStrongWind,
  WiThermometer
} from "react-icons/wi";
import "../styles/WeatherCard.css";

interface WeatherData {
  temperature: number;
  description: string;
  humidity: number;
  windSpeed: number;
  sunlightIntensity: number;
  dailyEvaporation: number;
  vpd: number;
}

interface WeatherInfoProps {
  latitude: number;
  longitude: number;
}

export const WeatherInfo: React.FC<WeatherInfoProps> = ({ latitude, longitude }) => {
  const [weather, setWeather] = useState<WeatherData | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>("");

  const getWeatherIcon = (description: string) => {
    const desc = description.toLowerCase();
    if (desc.includes("clear")) return <WiDaySunny size={64} />;
    if (desc.includes("partly cloudy")) return <WiCloudy size={64} />;
    if (desc.includes("overcast")) return <WiCloud size={64} />;
    if (desc.includes("fog")) return <WiFog size={64} />;
    if (desc.includes("rain") || desc.includes("drizzle")) return <WiRain size={64} />;
    if (desc.includes("snow")) return <WiSnow size={64} />;
    return <WiCloudy size={64} />;
  };

  const getVpdStatus = (vpd: number) => {
    if (vpd < 0.4) return { text: "Low Stress", class: "status-info" };
    if (vpd <= 0.8) return { text: "Optimal", class: "status-optimal" };
    if (vpd <= 1.2) return { text: "Moderate Stress", class: "status-warning" };
    return { text: "High Stress", class: "status-danger" };
  };

  const getEvaporationStatus = (evap: number) => {
    if (evap < 2) return { text: "Low Water Loss", class: "status-info" };
    if (evap <= 4) return { text: "Moderate Water Loss", class: "status-warning" };
    return { text: "High Water Loss", class: "status-danger" };
  };

  const getSunlightStatus = (sun: number) => {
    if (sun < 10) return { text: "Low Light", class: "status-info" };
    if (sun <= 20) return { text: "Moderate Light", class: "status-optimal" };
    return { text: "High Intensity", class: "status-warning" };
  };

  useEffect(() => {
    const fetchWeather = async () => {
      setLoading(true);
      setError("");
      try {
        const res = await fetch(
          `${BACKEND_BASE_URL}/weather/current?latitude=${latitude}&longitude=${longitude}`
        );
        if (!res.ok) throw new Error("Failed to fetch weather data");

        const data = await res.json();

        setWeather({
          temperature: Math.round(data.temperature_2m),
          description: data.description,
          humidity: data.relative_humidity_2m,
          windSpeed: Math.round(data.wind_speed_10m),
          sunlightIntensity: Math.round(data.sunlight_intensity),
          dailyEvaporation: Math.round(data.daily_evaporation * 10) / 10,
          vpd: Math.round(data.vapour_pressure_deficit * 100) / 100,
        });
      } catch (err: any) {
        console.error("Weather fetch error:", err);
        setError("Failed to sync weather station");
      } finally {
        setLoading(false);
      }
    };

    fetchWeather();
  }, [latitude, longitude]);

  if (loading) {
    return (
      <div className="weather-card-container weather-skeleton">
        <div className="weather-main-section">
          <div className="weather-main-info">
            <div className="skeleton-circle"></div>
            <div>
              <div className="skeleton-text" style={{ width: "100px", height: "3rem" }}></div>
              <div className="skeleton-text" style={{ width: "150px" }}></div>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (error || !weather) return null;

  const vpdInfo = getVpdStatus(weather.vpd);
  const evapInfo = getEvaporationStatus(weather.dailyEvaporation);
  const sunInfo = getSunlightStatus(weather.sunlightIntensity);

  return (
    <div className="weather-card-container">
      <div className="weather-main-section">
        <div className="weather-main-info">
          <div className="weather-icon-wrapper">
            {getWeatherIcon(weather.description)}
          </div>
          <div>
            <div className="weather-temp">{weather.temperature}°F</div>
            <div className="weather-desc">{weather.description}</div>
          </div>
        </div>
        <div className="weather-location">
          <div>Updated Just Now</div>
          <div style={{ color: '#4ade80' }}>● Live Weather Sync</div>
        </div>
      </div>

      <div className="weather-metrics-grid">
        <div className="weather-metric">
          <div className="metric-icon vpd"><WiBarometer /></div>
          <div className="metric-content">
            <div className="metric-label">Vapor Pressure Deficit</div>
            <div className="metric-value">
              {weather.vpd} <span className="metric-unit">kPa</span>
            </div>
            <div className={`metric-status ${vpdInfo.class}`}>{vpdInfo.text}</div>
          </div>
        </div>

        <div className="weather-metric">
          <div className="metric-icon evap"><WiThermometer /></div>
          <div className="metric-content">
            <div className="metric-label">Daily Evaporation (ET0)</div>
            <div className="metric-value">
              {weather.dailyEvaporation} <span className="metric-unit">mm</span>
            </div>
            <div className={`metric-status ${evapInfo.class}`}>{evapInfo.text}</div>
          </div>
        </div>

        <div className="weather-metric">
          <div className="metric-icon sun"><WiDaySunny /></div>
          <div className="metric-content">
            <div className="metric-label">Sunlight Intensity</div>
            <div className="metric-value">
              {weather.sunlightIntensity} <span className="metric-unit">MJ/m²</span>
            </div>
            <div className={`metric-status ${sunInfo.class}`}>{sunInfo.text}</div>
          </div>
        </div>

        <div className="weather-metric">
          <div className="metric-icon humid"><WiHumidity /></div>
          <div className="metric-content">
            <div className="metric-label">Relative Humidity</div>
            <div className="metric-value">
              {weather.humidity} <span className="metric-unit">%</span>
            </div>
            <div className="metric-status status-info">
              Wind: {weather.windSpeed} mph
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
