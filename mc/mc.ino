/******************************************************************************
 *  mc.ino — Sensorbox Hauptprogramm
 *
 *  Beschreibung:
 *  Der ESP32-C6 verbindet sich mit einem WLAN-Netzwerk und sendet die Messwerte
 *der angeschlossenen Sensoren in regelmässigen zeitlichenAbständen als
 *JSON-String an einen zentralen Server. Ist kein bekanntes Netzwerk verfügbar,
 *startet ein Captive Portal (Access Point z.B. "sensorbox1"), ueber das
 *WLAN-Credentials eingegeben werden koennen.
 *
 *  Board: Waveshare ESP32-C6-N8
 *  Die Logik der einzelnen Sensoren, Libraries und Wiring sind in .h-Dateien
 *ausgelagert. Bei I2C-Sensoren ist der I2C-Bus auf GPIO6 (SDA) und GPIO7 (SCL)
 *gelegt.
 *
 *  I2C Pull-Up: Ein Paar 2.2kOhm an SDA und SCL nach 3.3V
 * Initial müssen in Preferences der Lautstärkesensor INMP441
 *(../calibration/lautstaerke/01_calib.ino) und die Waage (über HX711 ->
 *../calibration/gewicht/01_calib.ino) vorbereitet und dann gespeichert werden.
 * Ebenfalls muss die box-id in Preferences (Namespace "sensorbox", Key "boxid")
 *gespeichert werden, damit das Captive Portal den richtigen Namen bekommt und
 *die Werte i die richtige DB-Tabelle gespeichert werden
 *(../calibration/box_id_speichern.ino).
 *
 * Im Captive Portal Modus hostet der ESP32-C6 einen Webserver, der die
 *HTML/CSS/JS-Dateien aus dem LittleFS-Dateisystem bereitstellt. Die Dateien
 *können  mit der Arduino IDE hochgeladen werden. Der ESP zeigt dann die
 *Konfigurations-Website an, die im Verzeichnis data liegt (html, css, js). Hier
 *können die Credentials des WLAN Netzwerks mit Internet hineterlegt werden, mit
 *dem sich der ESP32-C6 verbinden soll. Am Gerät befinden sich 2 Buttons:
 * - Reset (Neustart) und
 * - Captive Portal (hier wird erstellt der ESP ein Netzwerk, mit dem sich der
 *Benutzer verbinden kann, um die WLAN-Credentials zu hinterlegen). Wenn fertig,
 *Reset drücken.

 * Falls Programm zu gross für Compiler: Wähle Tools > Partition Scheme > Huge
 APP (3MB No OTA/1MB SPIFFS)
 *****************************************************************************/

#include "FS.h"
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <Wire.h>

// ======================== Pin-Definitionen ========================

#define I2C_SDA 6
#define I2C_SCL 7
#define PIN_BUTTON_PORTAL 20
#define PIN_BUTTON_DISPLAY 10
#define PIN_LED BUILTIN_LED

int prevTimestamp = 0;
int intervall_ms = 15000; // 15s -  Mess-Intervalle
int boxid = 0;
float temp_offset = 0.0;
JSONVar latestData;

#include "display.h" // SSD1306 OLED Display
#include "wlan.h" // WLAN Verbindung & Captive Portal (definiert server, preferences etc.)

////////////////////////// Sensor-Header einbinden

#include "alkohol.h"                         // MQ-3 Alkoholsensor
#include "bewegung.h"                        // SR602 PIR
#include "co2_temperatur_luftfeuchtigkeit.h" // SCD41 CO2, Temperatur, Luftfeuchtigkeit
#include "distanz.h"                         // VL53L0X Distanz
#include "gewicht.h"                         // HX711 Waage
#include "gps.h"                             // NEO8M
#include "helligkeit.h"                      // BH1750 Lichtsensor
#include "lage.h"                            // ICM-20948 9DOF
#include "lautstaerke.h"                     // INMP441 Mikrofon
#include "luftdruck.h"                       // BMP280 Luftdruck
#include "magnet.h"                          // GPS14-B Magnetsensor

////////////////////////// LittleFS Datei-Handler

bool handleFileRead(
    String path) // wird aufgerufen in wlan.h: Website-Dateien für Captive
                 // Portal aus LittleFS bereitstellen
{
  if (path.endsWith("/"))
    path += "index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html"))
    contentType = "text/html";
  else if (path.endsWith(".css"))
    contentType = "text/css";
  else if (path.endsWith(".js"))
    contentType = "application/javascript";
  else if (path.endsWith(".ico"))
    contentType = "image/x-icon";
  else if (path.endsWith(".json"))
    contentType = "application/json";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  rgbLedWrite(PIN_LED, 0, 255, 0); // Rot (keine WLAN Verbindung)

  preferences.begin("sensorbox", true);
  boxid = preferences.getInt("boxid", 0);
  int intervall_s = preferences.getInt("intervall_s", 15);
  intervall_ms = intervall_s * 1000;
  temp_offset = preferences.getFloat("temp_offset", 0.0);
  float waage_cal = preferences.getFloat("waage_cal", 1.0);
  float dboffset = preferences.getFloat("dboffset", 120.0);
  preferences.end();

  Serial.println("\n--- Gelesene Einstellungen (Preferences) ---");
  Serial.printf("Box ID: %d\n", boxid);
  Serial.printf("Messintervall: %d Sekunden\n", intervall_s);
  Serial.printf("Temperatur-Offset: %.1f °C\n", temp_offset);
  Serial.printf("Gewicht-Faktor: %.4f\n", waage_cal);
  Serial.printf("Mikrofon-Offset: %.1f dB\n", dboffset);
  Serial.println("--------------------------------------------\n");

  Wire.begin(I2C_SDA, I2C_SCL);
  setupDisplay();                     // in display.h
  displayText("Starte Sensorbox..."); // in display.h

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount fehlgeschlagen!");
    displayText("LittleFS Fehler!"); // in display.h
  }

  Serial.println("\n--------\nInitialisiere Sensoren...");
  displayText("Sensoren\ninitialisieren..."); // NEW
  setupAlkohol();                             // in alkohol.h
  setupBewegung();                            // in bewegung.h
  setupCo2_Temperatur_Luftfeuchtigkeit(); // co2_temperatur_luftfeuchtigkeit.h
  setupDistanz();                         // in distanz.h
  setupGewicht();                         // in gewicht.h
  setupGPS();                             // in gps.h
  setupHelligkeit();                      // in helligkeit.h
  setupLage();                            // in lage.h
  setupLautstaerke();                     // in lautstaerke.h
  setupLuftdruck();                       // in luftdruck.h
  setupMagnet();                          // in magnet.h
  Serial.println("\n-------- Initialisierung der Sensoren abgeschlossen...");

  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLDOWN);
  setupPortalButton(); // in wlan.h
  setupWLAN();         // in wlan.h
  Serial.println("\n-------- Setup abgeschlossen...");
  Serial.println("\n-------- Sensorwerte alle " + String(intervall_ms) +
                 " ms abfragen...");
}

void loop() {
  JSONVar dataObject;

  maintainWiFiConnection(); // in wlan.h
  checkDisplayButton();     // in display.h

  if (millis() < prevTimestamp + intervall_ms)
    return;
  prevTimestamp = millis();

  float alkohol = getAlkohol(); // in alkohol.h
  Serial.printf("Alkohol: %.2f mg/L\n", alkohol);
  dataObject["alkohol"] = alkohol;

  int bewegung = getBewegung(); // in bewegung.h
  Serial.printf("Bewegung: %d\n", bewegung);
  dataObject["bewegung"] = bewegung;

  getCo2_Temperatur_Luftfeuchtigkeit(); // in co2_temperatur_luftfeuchtigkeit.h
  Serial.printf("CO2: %d ppm\n", co2);
  Serial.printf("Temperatur: %.2f °C\n", temperature);
  Serial.printf("Luftfeuchtigkeit: %.2f %%\n", humidity);
  dataObject["co2"] = co2;
  dataObject["temperatur"] = temperature;
  dataObject["luftfeuchtigkeit"] = humidity;

  int distanz = getDistanz(); // in distanz.h
  Serial.printf("Distanz: %d\n", distanz);
  dataObject["distanz"] = distanz;

  float gewicht = getGewicht(); // in gewicht.h
  Serial.printf("Gewicht: %.2f g\n", gewicht);
  dataObject["gewicht"] = gewicht;

  getGPS(); // in gps.h
  Serial.printf("Latitude: %.6f°\n", latitude);
  Serial.printf("Longitude: %.6f°\n", longitude);
  Serial.printf("Altitude: %.2f m\n", altitude);
  Serial.printf("Satelliten: %d\n", satellites);
  Serial.printf("timeString: %s\n", timeString.c_str());
  dataObject["latitude"] = latitude;
  dataObject["longitude"] = longitude;
  dataObject["altitude"] = altitude;
  dataObject["gps_num_satellites"] = satellites;
  dataObject["gps_time"] = timeString;

  float helligkeit = getHelligkeit(); // in helligkeit.h
  Serial.printf("Helligkeit: %.2f lux\n", helligkeit);
  dataObject["helligkeit"] = helligkeit;

  getLage(); // in lage.h
  Serial.printf("lage_x: %.2f\n", lage_x);
  Serial.printf("lage_y: %.2f\n", lage_y);
  dataObject["lage_x"] = lage_x;
  dataObject["lage_y"] = lage_y;

  float lautstaerke = getLautstaerke(); // in lautstaerke.h
  Serial.printf("Lautstärke: %.2f dB\n", lautstaerke);
  dataObject["lautstaerke"] = lautstaerke;

  float luftdruck = getLuftdruck(); // in luftdruck.h
  Serial.printf("Luftdruck: %.2f hPa\n", luftdruck);
  dataObject["luftdruck"] = luftdruck;

  int magnet = getMagnet(); // in magnet.h
  Serial.printf("Magnet: %d\n", magnet);
  dataObject["magnet"] = magnet;

  ////////////////////////// Sensorwerte in JSON-String codieren");
  dataObject["boxid"] = boxid;
  latestData = dataObject;
  renderCurrentPage();

  String jsonString = JSON.stringify(dataObject);
  // Serial.println(jsonString);

  ////////////////////////// JSON string per HTTP POST Request an den Server
  /// schicken (server2db.php)

  String serverURL = "https://sensorbox.fiessling.ch/api/set.php";

  if (WiFi.status() == WL_CONNECTED) { // Überprüfen, ob Wi-Fi verbunden ist
    // HTTP Verbindung starten und POST-Anfrage senden
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

    // Prüfen der Antwort vom Server
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      Serial.println("Response: " + response);
    } else {
      Serial.printf("Error on sending POST: %d\n", httpResponseCode);
    }

    http.end();
    Serial.println("\n-----------------------------\n");
  } else {
    Serial.println("WiFi Disconnected");
  }
}
