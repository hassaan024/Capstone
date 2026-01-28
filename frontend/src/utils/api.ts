import { BACKEND_BASE_URL } from "./constants";

// API helper functions
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
