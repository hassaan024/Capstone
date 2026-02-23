
// Note: ALL THESE UNITS ARE WRT a day
export interface WeatherInfoDto{

  //------------------------------Air conditions---------------------------------------------
  // note this mapp contains my names and untis for each field
  //      MY STUFF          |  API STUFF

  // temperature_2m         | temperature_2m
  //      °F                |      °C
  temperature_2m?: number; // temp 2m above the ground

  // realtive_humidity_2m   | realtive_humidity_2m
  //      %                 |      %
  relative_humidity_2m?: number; // relative temp 2m above the ground

  // wind_speed_10m         | wind_speed_10m
  //      mph               |     m/s
  wind_speed_10m?: number; // wind speed 10m above the ground

  //-----------------------------------Sun---------------------------------------------------
  //sunlight_intensity      | shortwave_radiation
  //    W/m²                |        W/m²
  sunlight_intensity?: number; // W/m² → watts per square meter ##########

  //-------------------------------Water balance------------------------------------------
  // precipitation          |  precipitation
  //      mm                |      mm
  precipitation?: number; // mm = 1 liter of water per square meter of surface.

  // daily_evaporatoin      | et0_fao_evapotranspiration
  //      mm                |        mm
  daily_evaporation?: number; 

  //-------------------------------Plant stress indicator----------------------------------
  // How strongly the air is pulling water out of the plant right now.
  // VPD = (maximum moisture air could hold at that temperature) - (actual moisture currently in the air)
  //vapour_pressure_deficit | vapour_pressure_deficit
  //          kPa           |       kPa    
  vapour_pressure_deficit?: number; // kPa (kilopascal, a unit of pressure)

  //---------------------------------Soil conditions-----------------------------------------
  // soil_temperature_6cm   | soil_temperature_6cm
  //        °F              |         °C
  soil_temperature_6cm?: number; // °C

  // m³/m³ volumetric water content of soil in top 3 cm
  // (Volume of water in soil) / (Total volume of soil (water + air + water))
  //soil_moisture_0_to_3cm  | soil_moisture_0_to_3cm
  //        m³/m³           |        m³/m³
  soil_moisture_0_to_3cm?: number; 

  //---------------------------------Timezone info--------------------------------------------
  //        timezone        |         timezone
  //        str             |         str
  timezone?: string; // e.g. "America/Chicago"

  // field tells you how many seconds your local time is ahead of or behind UTC (Coordinated Universal Time)
  //   utc_offset_seconds   |         utc_offset_seconds
  //       sec              |             sec
  utc_offset_seconds?: number; // ex: -21600

  //  description           | retreved from weather_code
  //    str                 |          int
  description?: string;

  //    elevation           |     elevation
  //    m                   |          m
  elevation?: number; // meters above/below sea level
}