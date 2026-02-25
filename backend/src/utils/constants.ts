// model api 
export const PLANT_MODEL_PORT = 8000;

// weather api base url
export const OPEN_METEO_CURRENT_FORCAST_URL =
  'https://historical-forecast-api.open-meteo.com/v1/forecast';
export const OPEN_METEO_ARCHIVE_FORCAST_URL =
  'https://archive-api.open-meteo.com/v1/era5';
// React client info
export const REACT_GOOGLE_AUTH_URI = process.env.REACT_GOOGLE_AUTH_URI;
export const REACT_GOOGLE_TOKEN_URI = process.env.REACT_GOOGLE_TOKEN_URI;
export const REACT_GOOGLE_CLIENT_ID = process.env.REACT_GOOGLE_CLIENT_ID;
export const REACT_GOOGLE_CLIENT_SECRET =
  process.env.REACT_GOOGLE_CLIENT_SECRET;
export const REACT_REDIRECT_URI = process.env.REACT_REDIRECT_URI;

// Unreal client info
export const UNREAL_GOOGLE_AUTH_URI = process.env.UNREAL_GOOGLE_AUTH_URI;
export const UNREAL_GOOGLE_TOKEN_URI = process.env.UNREAL_GOOGLE_TOKEN_URI;
export const UNREAL_GOOGLE_CLIENT_ID = process.env.UNREAL_GOOGLE_CLIENT_ID;
export const UNREAL_GOOGLE_CLIENT_SECRET =
  process.env.UNREAL_GOOGLE_CLIENT_SECRET;
export const UNREAL_REDIRECT_URI = process.env.UNREAL_REDIRECT_URI;

// Grouped for convenience
export const GOOGLE_CLIENTS = {
  REACT: {
    AUTH_URI: REACT_GOOGLE_AUTH_URI,
    TOKEN_URI: REACT_GOOGLE_TOKEN_URI,
    CLIENT_ID: REACT_GOOGLE_CLIENT_ID,
    CLIENT_SECRET: REACT_GOOGLE_CLIENT_SECRET,
    REDIRECT_URI: REACT_REDIRECT_URI,
  },
  UNREAL: {
    AUTH_URI: UNREAL_GOOGLE_AUTH_URI,
    TOKEN_URI: UNREAL_GOOGLE_TOKEN_URI,
    CLIENT_ID: UNREAL_GOOGLE_CLIENT_ID,
    CLIENT_SECRET: UNREAL_GOOGLE_CLIENT_SECRET,
    REDIRECT_URI: UNREAL_REDIRECT_URI,
  },
};
