/* ------------------------------------------------------------------
   script.js — API Box Dashboard
   Wird per LittleFS hochgeladen, eingebunden in index.html.
   Ruft alle Sensor-Endpunkte per fetch() ab und zeigt sie als Karten.
------------------------------------------------------------------ */

// Alle verfuegbaren Endpunkte (Pfad + Anzeigename)
const ENDPOINTS = [
  { path: "/temperatur",       label: "Temperatur" },
  { path: "/luftfeuchtigkeit", label: "Luftfeuchtigkeit" },
  { path: "/co2",              label: "CO2" },
  { path: "/bewegung",         label: "Bewegung" },
  { path: "/lautstaerke",      label: "Lautstärke" },
  { path: "/magnet",           label: "Magnet / Tür" },
  { path: "/helligkeit",       label: "Helligkeit" },
  { path: "/alkohol",          label: "Alkohol" },
  { path: "/lage",             label: "Lage (9DOF)" },
  { path: "/gewicht",          label: "Gewicht" },
  { path: "/rauch",            label: "Gas / Rauch" },
  { path: "/luftdruck",        label: "Luftdruck" },
  { path: "/hoehe",            label: "Höhe" },
  { path: "/distanz",          label: "Distanz" }
];

const grid   = document.getElementById("grid");
const status = document.getElementById("status");

// Karten initial anlegen
const cards = {};
ENDPOINTS.forEach(ep => {
  const card = document.createElement("div");
  card.className = "card";
  card.innerHTML = `
    <div class="name">${ep.label}</div>
    <div class="value">–</div>
    <div class="unit"></div>
    <div class="sensor"></div>
    <div class="endpoint">${ep.path}</div>`;
  grid.appendChild(card);
  cards[ep.path] = card;
});

// Einen Wert formatieren (Objekt bei /lage, sonst Zahl/Boolean)
function formatValue(wert) {
  if (wert !== null && typeof wert === "object") {
    return `x ${wert.x} · y ${wert.y} · z ${wert.z}`;
  }
  if (typeof wert === "boolean") {
    return wert ? "JA" : "NEIN";
  }
  return wert;
}

// Einen einzelnen Endpunkt abfragen
async function fetchEndpoint(ep) {
  const card = cards[ep.path];
  try {
    const res  = await fetch(ep.path);
    const data = await res.json();

    card.classList.remove("error");
    card.querySelector(".value").textContent  = formatValue(data.wert);
    card.querySelector(".unit").textContent   = data.einheit || "";
    card.querySelector(".sensor").textContent = data.sensor ? "Sensor: " + data.sensor : "";
  } catch (e) {
    card.classList.add("error");
    card.querySelector(".value").textContent = "Fehler";
    card.querySelector(".unit").textContent  = "";
  }
}

// Alle Endpunkte abfragen
async function refreshAll() {
  status.textContent = "Lade Werte...";
  await Promise.all(ENDPOINTS.map(fetchEndpoint));
  const now = new Date();
  status.textContent = "Stand: " + now.toLocaleTimeString();
}

document.getElementById("refresh").addEventListener("click", refreshAll);

// Erstabfrage + automatische Aktualisierung alle 5 Sekunden
refreshAll();
setInterval(refreshAll, 5000);
