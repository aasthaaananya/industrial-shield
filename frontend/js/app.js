const DEMO_LIMITS = { gasWarning: 1500, gasCritical: 2500, tempWarning: 55, tempCritical: 70 };
let demoGas = 420;
let demoTemperature = 38.5;
let previousRisk = "SAFE";
let alertEvents = [];
let demoTimer = null;

function assessRisk(gas, temperature) {
  if (gas >= DEMO_LIMITS.gasCritical || temperature >= DEMO_LIMITS.tempCritical) {
    return { level: "CRITICAL", description: "One or more readings crossed a critical demo limit" };
  }
  if (gas >= DEMO_LIMITS.gasWarning || temperature >= DEMO_LIMITS.tempWarning) {
    return { level: "WARNING", description: "Unusual readings detected; inspection recommended" };
  }
  return { level: "SAFE", description: "No immediate hazards detected" };
}

function updateRisk(risk) {
  const status = document.getElementById("risk-status");
  const dot = document.getElementById("risk-dot");
  status.textContent = risk.level;
  status.className = `${risk.level.toLowerCase()}-text`;
  dot.className = `risk-dot ${risk.level.toLowerCase()}-bg`;
  document.getElementById("risk-description").textContent = risk.description;
  document.getElementById("risk-caption").textContent =
    risk.level === "SAFE" ? "Conditions within demo limits" : "Check sensor readings and local response";
}

function addAlert(risk, reading) {
  if (risk.level === "SAFE" || risk.level === previousRisk) return;
  const event = {
    level: risk.level,
    message: risk.description,
    timestamp: reading.timestamp || new Date().toISOString()
  };
  alertEvents.unshift(event);
  alertEvents = alertEvents.slice(0, 8);
  renderAlerts();
}

function renderAlerts() {
  const list = document.getElementById("alert-list");
  const count = alertEvents.filter(a => a.level !== "SAFE").length;
  document.getElementById("alert-count").textContent = count;
  if (!alertEvents.length) {
    list.innerHTML = '<div class="empty-alert">No alerts recorded. System operating normally.</div>';
    return;
  }
  list.innerHTML = alertEvents.map(event => {
    const time = new Date(event.timestamp).toLocaleString();
    return `<div class="alert-item">
      <div class="alert-mark ${event.level === "CRITICAL" ? "critical" : ""}">⚠</div>
      <div class="alert-info"><strong>${event.level}: ${event.message}</strong><small>Risk status changed</small></div>
      <span class="alert-time">${time}</span>
    </div>`;
  }).join("");
}

function renderReading(reading, source = "demo") {
  const gas = Number(reading.gas_value);
  const temp = Number(reading.temperature);
  if (!Number.isFinite(gas) || !Number.isFinite(temp)) return;

  document.getElementById("gas-value").textContent = Math.round(gas);
  document.getElementById("gas-reading").textContent = Math.round(gas);
  document.getElementById("temperature-value").textContent = temp.toFixed(1);
  document.getElementById("temp-reading").textContent = temp.toFixed(1);
  document.getElementById("gas-progress").style.width = `${Math.min(100, Math.max(0, gas / 4095 * 100))}%`;
  document.getElementById("temp-progress").style.width = `${Math.min(100, Math.max(0, temp / 100 * 100))}%`;

  const gasCondition = gas >= DEMO_LIMITS.gasCritical ? "Critical" : gas >= DEMO_LIMITS.gasWarning ? "Elevated" : "Normal";
  const tempCondition = temp >= DEMO_LIMITS.tempCritical ? "Critical" : temp >= DEMO_LIMITS.tempWarning ? "Elevated" : "Normal";
  document.getElementById("gas-condition").textContent = gasCondition;
  document.getElementById("temp-condition").textContent = tempCondition;
  document.getElementById("gas-summary").textContent = `${gasCondition} demo range`;
  document.getElementById("temp-summary").textContent = `${tempCondition} demo range`;

  const risk = reading.risk || assessRisk(gas, temp);
  addAlert(risk, reading);
  updateRisk(risk);
  previousRisk = risk.level;

  document.getElementById("relay-status").textContent = reading.relay_on ? "ON" : "OFF";
  document.getElementById("relay-status").className = `device-status ${reading.relay_on ? "on" : ""}`;
  document.getElementById("buzzer-status").textContent = reading.buzzer_on ? "ON" : "OFF";
  document.getElementById("buzzer-status").className = `device-status ${reading.buzzer_on ? "on" : ""}`;
  document.getElementById("esp-status").textContent = source === "backend" ? "ONLINE" : "DEMO";
  document.getElementById("esp-status").className = `device-status ${source === "backend" ? "online" : ""}`;
  document.getElementById("connection-label").textContent = source === "backend" ? "Backend connected" : "Demo mode";
  document.getElementById("connection-detail").textContent = source === "backend" ? "API data received" : "Simulated sensor data";
  document.getElementById("mode-label").textContent = source === "backend" ? "LIVE DATA" : "DEMO LIVE";
  document.getElementById("last-updated").textContent = `Updated ${new Date().toLocaleTimeString()}`;

  addChartReading({ ...reading, timestamp: reading.timestamp || new Date().toISOString() });
}

function demoUpdate() {
  // Gentle variation keeps the interface active without implying real sensor data.
  demoGas = Math.max(250, Math.min(900, demoGas + (Math.random() - 0.5) * 100));
  demoTemperature = Math.max(32, Math.min(45, demoTemperature + (Math.random() - 0.5) * 1.4));
  renderReading({
    gas_value: Math.round(demoGas),
    temperature: Number(demoTemperature.toFixed(1)),
    relay_on: false,
    buzzer_on: false,
    timestamp: new Date().toISOString()
  }, "demo");
}

async function backendUpdate() {
  try {
    const reading = await getLatestReading();
    renderReading(reading, "backend");
  } catch (error) {
    document.getElementById("connection-label").textContent = "Backend unavailable";
    document.getElementById("connection-detail").textContent = "Check FastAPI server";
    document.getElementById("mode-label").textContent = "API OFFLINE";
    console.error(error);
  }
}

function startDashboard() {
  document.getElementById("current-date").textContent = new Date().toLocaleDateString(undefined, {
    weekday: "short", day: "2-digit", month: "short", year: "numeric"
  });
  createSensorChart();

  if (USE_BACKEND) {
    backendUpdate();
    demoTimer = setInterval(backendUpdate, 2000);
  } else {
    demoUpdate();
    demoTimer = setInterval(demoUpdate, 2000);
  }
}

document.addEventListener("DOMContentLoaded", startDashboard);
