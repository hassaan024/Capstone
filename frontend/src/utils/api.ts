// API configuration
export const API_BASE_URL = 'http://localhost:4000/backend';

// API helper functions
// API helper functions
const get = async (url: string) => {
  const response = await fetch(`${API_BASE_URL}${url}`);
  if (!response.ok) {
    throw new Error(`API Error: ${response.statusText}`);
  }
  return { data: await response.json() };
};

const post = async (url: string, body: any) => {
  const response = await fetch(`${API_BASE_URL}${url}`, {
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

export const api = { get, post };
export const apiRequest = async (endpoint: string, options: RequestInit = {}) => {
    return fetch(`${API_BASE_URL}${endpoint}`, options);
};
