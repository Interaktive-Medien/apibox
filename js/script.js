console.log("huhu");

async function loadData() {
  try {
    const response = await fetch("api/unload.php");
    const jsonData = await response.json();
    console.log(jsonData);
  } catch (error) {
    console.error("Fehler beim Laden der Daten:", error);
  }
}

loadData();
