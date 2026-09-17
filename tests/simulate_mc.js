console.log("simulate_mc.js wurde geladen!");

console.log("simulate_mc.js wurde geladen!");

const form = document.querySelector("#sensorForm");
if (form) {
  console.log("Formular gefunden!");
  form.addEventListener("submit", function (e) {
    e.preventDefault(); // Verhindert das klassische Neuladen der Seite
    console.log("Formular wurde abgesendet!");
    console.log("Formular wurde abgesendet!");

    // Lese Daten aus dem Formular aus
    const testData = {
      boxid: parseInt(document.querySelector("#boxid").value, 10),
      temperatur: parseFloat(document.querySelector("#temperatur").value),
      luftfeuchtigkeit: parseFloat(document.querySelector("#luftfeuchtigkeit").value),
      bewegung: parseInt(document.querySelector("#bewegung").value, 10),
      lautstaerke: parseFloat(document.querySelector("#lautstaerke").value),
      magnet: parseInt(document.querySelector("#magnet").value, 10),
      helligkeit: parseFloat(document.querySelector("#helligkeit").value),
      alkohol: parseFloat(document.querySelector("#alkohol").value),
      lage_x: parseFloat(document.querySelector("#lage_x").value),
      lage_y: parseFloat(document.querySelector("#lage_y").value),
      gewicht: parseFloat(document.querySelector("#gewicht").value),
      co2: parseInt(document.querySelector("#co2").value, 10),
      luftdruck: parseFloat(document.querySelector("#luftdruck").value),
      distanz: parseInt(document.querySelector("#distanz").value, 10),
      latitude: parseFloat(document.querySelector("#latitude").value),
      longitude: parseFloat(document.querySelector("#longitude").value),
      altitude: parseFloat(document.querySelector("#altitude").value),
      gps_time: document.querySelector("#gps_time").value,
      gps_num_satellites: parseInt(document.querySelector("#gps_num_satellites").value, 10),
    };

    const responseDiv = document.querySelector("#response");
    responseDiv.innerText = "Sende Daten an Server...";
    responseDiv.style.color = "black";

    // fetch API nutzen, um Daten an set.php zu schicken
    // Wir nehmen an, dass set.php unter ../api/set.php erreichbar ist
    fetch(`../api/set.php`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(testData),
    })
      .then(async (response) => {
        const text = await response.text();
        if (!response.ok) {
          throw new Error(
            `HTTP-Fehler! Status: ${response.status}\nAntwort vom PHP-Skript: ${text}`,
          );
        }
        try {
          return JSON.parse(text);
        } catch (e) {
          return text;
        }
      })
      .then((data) => {
        // Erfolgreiche Antwort vom Server anzeigen
        responseDiv.innerText = "Erfolg:\n" + JSON.stringify(data, null, 2);
        responseDiv.style.color = "green";
      })
      .catch((error) => {
        // Fehler anzeigen
        responseDiv.innerText = "Fehler aufgetreten:\n" + error.message;
        responseDiv.style.color = "red";
      });
  });
}
