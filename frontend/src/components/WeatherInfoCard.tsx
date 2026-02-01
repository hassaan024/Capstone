import React, { useEffect, useState } from "react";
import { BACKEND_BASE_URL } from "../utils/constants";
import {
  WiDaySunny,
  WiCloudy,
  WiCloud,
  WiFog,
  WiRain,
  WiSnow,
} from "react-icons/wi";

interface WeatherData {
  temperature: number;
  description: string;
  humidity: number;
  windSpeed: number;
}

interface WeatherInfoProps {
  latitude: number;
  longitude: number;
}

export const WeatherInfo: React.FC<WeatherInfoProps> = ({ latitude, longitude }) => {
  const [weather, setWeather] = useState<WeatherData | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>("");

  // Map description to React icon
  const getWeatherIcon = (description: string) => {
    const desc = description.toLowerCase();
    if (desc.includes("clear")) return <WiDaySunny size={48} />;
    if (desc.includes("partly cloudy")) return <WiCloudy size={48} />;
    if (desc.includes("overcast")) return <WiCloud size={48} />;
    if (desc.includes("fog")) return <WiFog size={48} />;
    if (desc.includes("rain") || desc.includes("drizzle")) return <WiRain size={48} />;
    if (desc.includes("snow")) return <WiSnow size={48} />;
    return <WiCloud size={48} />;
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
          temperature: data.temperature,
          description: data.description,
          humidity: data.humidity,
          windSpeed: data.windSpeed,
        });
      } catch (err: any) {
        console.error("Weather fetch error:", err);
        setError(err.message || "Failed to fetch weather");
      } finally {
        setLoading(false);
      }
    };

    fetchWeather();
  }, [latitude, longitude]);

  if (loading) return <div className="dashboard-card">Loading weather...</div>;
  if (error) return <div className="dashboard-card">Error: {error}</div>;
  if (!weather) return null;

  return (
    <div className="dashboard-card">
      <h2 className="dashboard-card-title">Current Weather</h2>
      <div className="flex items-center gap-4">
        {getWeatherIcon(weather.description)}
        <div>
          <div className="text-xl font-bold">{weather.temperature}°F</div>
          <div className="capitalize">{weather.description}</div>
          <div className="text-sm text-gray-500">
            Humidity: {weather.humidity}% | Wind: {weather.windSpeed} mph
          </div>
        </div>
      </div>
    </div>
  );
};
