import { BACKEND_BASE_URL } from "./constants";

export const sendLocationToBackend = async (userId: number, latitude: number, longitude: number) => {
  console.log("TRYING TO SEND LOCATION TO BACKEND!");

  try {
    const res = await fetch(`${BACKEND_BASE_URL}/user/${userId}`, {
      method: 'PATCH',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ latitude, longitude }),
    });
    
    console.log(res);

    if (!res.ok) {
      throw new Error(`Server responded with status ${res.status}`);
    }

    const data = await res.json();
    console.log("Backend response:", data);
    return data; 
  } catch (err) {
    console.error("Error sending location to backend:", err);
    throw err; 
  }
};