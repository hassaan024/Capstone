import { BACKEND_BASE_URL, GEOCODING_API_BASE_URL, GEOCODING_API_KEY } from "./constants";

// API helper functions
const get = async (url: string) => {
  const response = await fetch(`${BACKEND_BASE_URL}${url}`);
  if (!response.ok) {
    throw new Error(`API Error: ${response.statusText}`);
  }
  return { data: await response.json() };
};

const post = async (url: string, body: any) => {
  const response = await fetch(`${BACKEND_BASE_URL}${url}`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(body),
  });
  if (!response.ok) {
    throw new Error(`API Error: ${response.statusText}`);
  }
  return { data: await response.json() };
};

const del = async (url: string) => {
  const response = await fetch(`${BACKEND_BASE_URL}${url}`, {
    method: 'DELETE',
  });
  if (!response.ok) {
    throw new Error(`API Error: ${response.statusText}`);
  }
  return { data: await response.json() };
};

export const api = { get, post, del };
export const apiRequest = async (endpoint: string, options: RequestInit = {}) => {
    return fetch(`${BACKEND_BASE_URL}${endpoint}`, options);
};


export const fetchLongLatFromZipAndCountry = async (zipOrPostalCode: string, country: string) => {
  const url = `${GEOCODING_API_BASE_URL}/json?q=${zipOrPostalCode},${country}&key=${GEOCODING_API_KEY}`

  const response = await fetch(url);
  
  if (!response.ok) {
    throw new Error(`API Error: ${response.statusText}`);
  }

  const data = await response.json();

  //  make sure the data object has a longitude and latitude
  if (!data.results || data.results.length === 0) {
    throw new Error("No coordinates found for this ZIP + country");
  }
  
  const { lat, lng } = data.results[0].geometry;
  return { lat, lng };
};