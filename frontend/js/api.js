// Set to true after the FastAPI backend is running and connected to the ESP32.
const USE_BACKEND = true;
const API_BASE_URL = "http://127.0.0.1:8000";

async function getLatestReading() {
  const response = await fetch(`${API_BASE_URL}/api/sensors/latest`);
  if (!response.ok) throw new Error("Could not fetch latest sensor reading");
  return response.json();
}

async function getHistory() {
  const response = await fetch(`${API_BASE_URL}/api/sensors/history?limit=30`);
  if (!response.ok) throw new Error("Could not fetch sensor history");
  return response.json();
}

async function getAlerts() {
  const response = await fetch(`${API_BASE_URL}/api/alerts`);
  if (!response.ok) throw new Error("Could not fetch alerts");
  return response.json();
}
