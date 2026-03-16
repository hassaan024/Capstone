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
import { FaMapMarkerAlt, FaChevronDown, FaChevronUp } from 'react-icons/fa';
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid } from "recharts";
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

interface ForecastData {
  day: string;
  maxTemp: number;
  minTemp: number;
  description: string;
  dailyEvaporation?: number;
  sunlightIntensity?: number;
  vpd?: number;
  hourlyTemps: { time: string; temp: number }[];
}

interface WeatherInfoProps {
  latitude?: number;
  longitude?: number;
  onSetLocationClick: () => void;
}

export const WeatherInfo: React.FC<WeatherInfoProps> = ({ latitude, longitude, onSetLocationClick }) => {
  const [weather, setWeather] = useState<WeatherData | null>(null);
  const [forecast, setForecast] = useState<ForecastData[]>([]);
  const [selectedDayIndex, setSelectedDayIndex] = useState<number>(0);
  const [isDetailsOpen, setIsDetailsOpen] = useState<boolean>(false);
  const [cityName, setCityName] = useState<string>("Locating...");
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
    if (latitude === undefined || longitude === undefined) {
      setLoading(false);
      return;
    }

    const fetchWeather = async () => {
      setLoading(true);
      setError("");
      try {
        // Fetch City Name (using BigDataCloud Free API)
        fetch(`https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=${latitude}&longitude=${longitude}&localityLanguage=en`)
          .then(res => res.json())
          .then(data => {
            if (data?.city || data?.locality) {
              setCityName(`${data.city || data.locality}, ${data.principalSubdivision || data.countryCode}`);
            } else {
              setCityName("Unknown Location");
            }
          })
          .catch(() => setCityName("Location Data Unavailable"));

        // Fetch Current Weather
        const currentRes = await fetch(`${BACKEND_BASE_URL}/weather/current?latitude=${latitude}&longitude=${longitude}`);
        if (!currentRes.ok) throw new Error("Failed to fetch current weather");
        const currentData = await currentRes.json();

        setWeather({
          temperature: Math.round(currentData.temperature_2m),
          description: currentData.description,
          humidity: currentData.relative_humidity_2m,
          windSpeed: Math.round(currentData.wind_speed_10m),
          sunlightIntensity: Math.round(currentData.sunlight_intensity),
          dailyEvaporation: Math.round(currentData.daily_evaporation * 10) / 10,
          vpd: Math.round(currentData.vapour_pressure_deficit * 100) / 100,
        });

        // Fetch Weekly & Hourly Forecast (Open-Meteo directly for UI icons, min/max, and 24h temp)
        const weeklyRes = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&daily=weather_code,temperature_2m_max,temperature_2m_min,et0_fao_evapotranspiration,shortwave_radiation_sum&hourly=temperature_2m,vapour_pressure_deficit&timezone=auto`);
        if (weeklyRes.ok) {
          const weeklyData = await weeklyRes.json();
          if (weeklyData.daily && weeklyData.daily.time) {
            
            // Map the WMO codes to descriptions
            const wmoMap: Record<number, string> = {
              0: 'Clear sky', 1: 'Partly cloudy', 2: 'Partly cloudy', 3: 'Overcast',
              45: 'Fog', 48: 'Fog', 51: 'Rain', 61: 'Rain', 71: 'Snow',
            };

            const chartData = weeklyData.daily.time.map((timeStr: string, index: number) => {
              const d = new Date(timeStr);
              d.setMinutes(d.getMinutes() + d.getTimezoneOffset());
              const dayName = d.toLocaleDateString("en-US", { weekday: 'long' });
              const code = weeklyData.daily.weather_code[index];
              
              const hourlyTempsForDay = [];
              let vpdSum = 0;
              for (let i = 0; i < 24; i++) {
                const hourIndex = index * 24 + i;
                const temp = weeklyData.hourly.temperature_2m[hourIndex];
                const vpd = weeklyData.hourly.vapour_pressure_deficit[hourIndex];
                vpdSum += vpd;
                const hourLabel = new Date(weeklyData.hourly.time[hourIndex]).toLocaleTimeString("en-US", { hour: 'numeric' });
                hourlyTempsForDay.push({ time: hourLabel, temp: Math.round(temp * 9 / 5 + 32) });
              }
              const avgVpd = Math.round((vpdSum / 24) * 100) / 100;

              return {
                day: index === 0 ? "Today" : dayName,
                maxTemp: Math.round(weeklyData.daily.temperature_2m_max[index] * 9/5 + 32),
                minTemp: Math.round(weeklyData.daily.temperature_2m_min[index] * 9/5 + 32),
                description: wmoMap[code] || 'Cloudy',
                dailyEvaporation: Math.round(weeklyData.daily.et0_fao_evapotranspiration[index] * 10) / 10,
                sunlightIntensity: Math.round(weeklyData.daily.shortwave_radiation_sum[index]),
                vpd: avgVpd,
                hourlyTemps: hourlyTempsForDay
              };
            });
            setForecast(chartData);
          }
        }

      } catch (err: any) {
        console.error("Weather fetch error:", err);
        setError("Failed to sync weather station");
      } finally {
        setLoading(false);
      }
    };

    fetchWeather();
  }, [latitude, longitude]);

  if (latitude === undefined || longitude === undefined) {
    return (
      <div className="weather-card-container empty-state">
        <div className="empty-state-content">
          <div className="empty-state-icon">🌍</div>
          <h3>Local Weather & Plant Metrics</h3>
          <p>Set your location to see personalized plant stress metrics, evaporation rates, and a 7-day forecast.</p>
          <button onClick={onSetLocationClick} className="ll-btn ll-btn-primary">
            <FaMapMarkerAlt /> Add Your Location
          </button>
        </div>
      </div>
    );
  }

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

  const currentVpd = selectedDayIndex === 0 ? weather.vpd : (forecast[selectedDayIndex]?.vpd ?? weather.vpd);
  const currentEvap = selectedDayIndex === 0 ? weather.dailyEvaporation : (forecast[selectedDayIndex]?.dailyEvaporation ?? weather.dailyEvaporation);
  const currentSun = selectedDayIndex === 0 ? weather.sunlightIntensity : (forecast[selectedDayIndex]?.sunlightIntensity ?? weather.sunlightIntensity);

  const displayTemp = selectedDayIndex === 0 ? `${weather.temperature}°F` : `High: ${forecast[selectedDayIndex]?.maxTemp}° Low: ${forecast[selectedDayIndex]?.minTemp}°`;
  const displayDesc = selectedDayIndex === 0 ? weather.description : forecast[selectedDayIndex]?.description;
  const displayIcon = selectedDayIndex === 0 ? weather.description : forecast[selectedDayIndex]?.description;
  const metricsTitle = forecast.length > 0 && forecast[selectedDayIndex] ? `${forecast[selectedDayIndex].day}'s Plant Metrics` : "Plant Metrics";

  const vpdInfo = getVpdStatus(currentVpd);
  const evapInfo = getEvaporationStatus(currentEvap);
  const sunInfo = getSunlightStatus(currentSun);

  return (
    <div className="weather-card-container">
      <div className="weather-main-section">
        <div className="weather-main-info">
          <div className="weather-icon-wrapper">
            {getWeatherIcon(displayIcon)}
          </div>
          <div>
            <div className="weather-temp">{displayTemp}</div>
            <div className="weather-desc">{displayDesc} {selectedDayIndex !== 0 && `(${forecast[selectedDayIndex]?.day})`}</div>
          </div>
        </div>
        <div className="weather-location" style={{ display: 'flex', flexDirection: 'column', alignItems: 'flex-end', gap: '0.5rem' }}>
          <div className="weather-city" style={{ color: '#cbd5e1', fontSize: '0.95rem', fontWeight: 600 }}>
            <FaMapMarkerAlt style={{ display: 'inline', marginRight: '4px', color: '#38bdf8' }}/> 
            {cityName}
          </div>
          <button onClick={onSetLocationClick} className="weather-change-location-btn">
             Change Location
          </button>
        </div>
      </div>

      <div className="weather-dashboard-layout">
        
        {forecast.length > 0 && (
          <div>
            <h4 className="forecast-title" style={{ paddingLeft: '0.2rem' }}>7-Day Forecast</h4>
            <div className="forecast-weekly-grid">
              {forecast.map((day, idx) => (
                <div 
                  key={idx} 
                  className={`forecast-day-card ${selectedDayIndex === idx ? 'active' : ''}`}
                  onClick={() => { setSelectedDayIndex(idx); setIsDetailsOpen(true); }}
                >
                  <div className="forecast-day-name">{day.day}</div>
                  <div className="forecast-day-icon">
                    {getWeatherIcon(day.description)}
                  </div>
                  <div className="forecast-day-temps">
                    <span className="forecast-max">{day.maxTemp}°</span>
                    <span className="forecast-min">{day.minTemp}°</span>
                  </div>
                </div>
              ))}
            </div>
            <div style={{ display: 'flex', justifyContent: 'center', marginTop: '0.8rem' }}>
              <button 
                onClick={() => setIsDetailsOpen(!isDetailsOpen)}
                className="ll-btn ll-btn-ghost"
                style={{ fontSize: '0.85rem', display: 'flex', alignItems: 'center', gap: '0.4rem', color: '#94a3b8' }}
              >
                {isDetailsOpen ? <><FaChevronUp/> Hide Details</> : <><FaChevronDown/> Show {forecast[selectedDayIndex]?.day}'s Details</>}
              </button>
            </div>
          </div>
        )}

        {isDetailsOpen && (
          <>
            <div>
              <h4 className="forecast-title" style={{ marginBottom: '1rem', paddingLeft: '0.2rem' }}>{metricsTitle}</h4>
              <div className="weather-metrics-grid">
                <div className="weather-metric">
                  <div className="metric-icon vpd"><WiBarometer /></div>
                  <div className="metric-content">
                    <div className="metric-label">Vapor Pressure Deficit</div>
                    <div className="metric-value">
                      {currentVpd} <span className="metric-unit">kPa</span>
                    </div>
                    <div className={`metric-status ${vpdInfo.class}`}>{vpdInfo.text}</div>
                  </div>
                </div>

                <div className="weather-metric">
                  <div className="metric-icon evap"><WiThermometer /></div>
                  <div className="metric-content">
                    <div className="metric-label">Daily Evaporation (ET0)</div>
                    <div className="metric-value">
                      {currentEvap} <span className="metric-unit">mm</span>
                    </div>
                    <div className={`metric-status ${evapInfo.class}`}>{evapInfo.text}</div>
                  </div>
                </div>

                <div className="weather-metric">
                  <div className="metric-icon sun"><WiDaySunny /></div>
                  <div className="metric-content">
                    <div className="metric-label">Sunlight Intensity</div>
                    <div className="metric-value">
                      {currentSun} <span className="metric-unit">MJ/m²</span>
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

            {forecast.length > 0 && selectedDayIndex !== null && forecast[selectedDayIndex] && (
              <div className="weather-forecast-container" style={{ marginTop: '0.5rem' }}>
                 <h4 className="forecast-title">{forecast[selectedDayIndex].day}'s Temperature</h4>
                 <div className="forecast-chart-wrapper" style={{ width: '100%', height: '150px' }}>
                    <ResponsiveContainer width="100%" height="100%">
                      <LineChart data={forecast[selectedDayIndex].hourlyTemps} margin={{ top: 10, right: 10, left: -30, bottom: 0 }}>
                        <CartesianGrid strokeDasharray="3 3" stroke="rgba(148, 163, 184, 0.15)" vertical={false} />
                        <XAxis dataKey="time" stroke="#94a3b8" fontSize={11} tickLine={false} axisLine={false} interval={3} />
                        <YAxis stroke="#94a3b8" fontSize={12} tickLine={false} axisLine={false} domain={['dataMin - 5', 'dataMax + 5']} />
                        <Tooltip 
                          contentStyle={{ backgroundColor: 'rgba(15, 23, 42, 0.9)', border: '1px solid rgba(148, 163, 184, 0.2)', borderRadius: '8px', color: '#fff' }}
                          itemStyle={{ color: '#38bdf8', fontWeight: 'bold' }}
                          labelStyle={{ color: '#94a3b8', marginBottom: '4px' }}
                          formatter={(value: any) => [`${value}°F`, 'Temp']}
                        />
                        <Line type="monotone" dataKey="temp" stroke="#38bdf8" strokeWidth={3} dot={false} activeDot={{ r: 6, fill: '#38bdf8' }} />
                      </LineChart>
                    </ResponsiveContainer>
                  </div>
              </div>
            )}
          </>
        )}
      </div>
    </div>
  );
};
