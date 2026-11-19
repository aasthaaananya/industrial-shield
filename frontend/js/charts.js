let sensorChart = null;

function createSensorChart() {
  const canvas = document.getElementById("sensorChart");
  if (!canvas || typeof Chart === "undefined") return;

  sensorChart = new Chart(canvas, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: "Gas / smoke (ADC)",
          data: [],
          borderColor: "#42c8d5",
          backgroundColor: "rgba(66,200,213,.10)",
          borderWidth: 2,
          pointRadius: 2,
          tension: 0.35,
          yAxisID: "y"
        },
        {
          label: "Temperature (°C)",
          data: [],
          borderColor: "#f2b84b",
          backgroundColor: "rgba(242,184,75,.08)",
          borderWidth: 2,
          pointRadius: 2,
          tension: 0.35,
          yAxisID: "y1"
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: "index", intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: { backgroundColor: "#172634", titleColor: "#fff", bodyColor: "#c5d4df" }
      },
      scales: {
        x: { grid: { color: "rgba(130,155,175,.08)" }, ticks: { color: "#8296a7", maxTicksLimit: 8 } },
        y: { position: "left", grid: { color: "rgba(130,155,175,.10)" }, ticks: { color: "#8296a7" }, title: { display: true, text: "Gas ADC", color: "#42c8d5" } },
        y1: { position: "right", grid: { drawOnChartArea: false }, ticks: { color: "#8296a7" }, title: { display: true, text: "Temperature °C", color: "#f2b84b" } }
      }
    }
  });
}

function addChartReading(reading) {
  if (!sensorChart) return;
  const time = new Date(reading.timestamp || Date.now()).toLocaleTimeString([], {hour: "2-digit", minute: "2-digit", second: "2-digit"});
  sensorChart.data.labels.push(time);
  sensorChart.data.datasets[0].data.push(reading.gas_value);
  sensorChart.data.datasets[1].data.push(reading.temperature);
  while (sensorChart.data.labels.length > 20) {
    sensorChart.data.labels.shift();
    sensorChart.data.datasets.forEach(dataset => dataset.data.shift());
  }
  sensorChart.update("none");
}
