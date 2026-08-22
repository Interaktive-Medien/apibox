/******************************************************************************
 *  mc.ino — API Box Hauptprogramm
 *  ESP32-C6 IoT Sensor API Server
 *
 *  Beschreibung:
 *  Der ESP32-C6 verbindet sich mit einem WLAN-Netzwerk und stellt einen HTTP-Webserver bereit.
 *  Ueber definierte API-Endpunkte koennen angeschlossene Sensoren per HTTP GET abgefragt werden.
 *  Die Antworten werden als JSON-Strings zurueckgegeben.
 *  Ist kein bekanntes Netzwerk verfuegbar, startet ein Captive Portal (Access Point "apibox"),
 *  ueber das WLAN-Credentials eingegeben werden koennen.
 *
 *  Board: Waveshare ESP32-C6-N8
 *  Arduino Libraries:
 *    - OLED DisplayAdafruit SSD1306 + Adafruit GFX + Adafruit BusIO
 *    - CO2-Sensor: Sensirion I2C SCD4x + Sensirion Core
 *    - Lichtsensor: BH1750 (Christopher Laws)
 *    - Lagesensor: ICM20948_WE (Wolfgang Ewald)
 *    - Waage: HX711 Arduino Library (Bogdan Necula)
 *    - Luftdruck-Sensor (Höhe)Adafruit BMP280 Library
 *    - TOF Distanzsensor:VL53L0X Library (Polulu)
 *    - LittleFS (eingebaut)
 *    - Preferences (eingebaut)
 *    - DNSServer (eingebaut)
 *
 *  Verkabelung / Pinbelegung:
 *  +-----------------------+-------------------------------+
 *  | ESP32-C6 Pin          | Komponente                    |
 *  +-----------------------+-------------------------------+
 *  | GPIO6 (SDA)           | I2C Bus (SSD1306, SCD41,      |
 *  |                       | BH1750, ICM-20948, BMP280,    |
 *  |                       | VL53L0X)                      |
 *  | GPIO7 (SCL)           | I2C Bus (siehe oben)          |
 *  | GPIO4                 | PIR Sensor SR602 (Input)      |
 *  | GPIO5                 | MQ-3 Alkoholsensor (Analog)   |
 *  | GPIO0                 | MQ-2 Rauchsensor (Analog)     |
 *  | GPIO11                | Magnetsensor GPS14-B (Input)  |
 *  | GPIO20                | Taster Captive Portal (PULLDOWN)|
 *  | GPIO2                 | INMP441 SCK                   |
 *  | GPIO13                | INMP441 SD (Data)             |
 *  | GPIO23                | INMP441 WS                    |
 *  | GPIO21                | HX711 DT (Data) — Waage       |
 *  | GPIO22                | HX711 SCK (Clock) — Waage     |
 *  | BUILTIN_LED (GPIO8)   | RGB Status-LED                |
 *  +-----------------------+-------------------------------+
 *
 *  I2C Pull-Up: Ein Paar 2.2-4.7kOhm an SDA und SCL nach 3.3V genuegt
 *  (die meisten Breakout-Boards haben bereits Pull-Ups onboard).
 *
 *  WICHTIG HX711 (Waage): Anders als im urspruenglichen Steckplan NICHT auf
 *  GPIO6/7 (das ist der I2C-Bus!), sondern auf GPIO21 (DT) / GPIO22 (SCK).
 *  HX711 ist kein I2C-Geraet und wuerde den I2C-Bus stoeren. Siehe gewicht.h.
 *
 *****************************************************************************/

#include <Wire.h>
#include "FS.h"
#include "LittleFS.h"

// ======================== Pin-Definitionen ========================

#define I2C_SDA 6
#define I2C_SCL 7
#define PIN_BUTTON_PORTAL 20
#define PIN_LED BUILTIN_LED

// ======================== Mess-Intervalle ========================

int prevTimestamp = 0;
int intervall_ms = 15000;

// ======================== Zentrale Header (vor den Sensoren) ========================

#include "display.h" // SSD1306 OLED Display
#include "wlan.h"    // WLAN Verbindung & Captive Portal (definiert server, preferences etc.)

// ======================== Sensor-Header einbinden ========================

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

// ======================== LittleFS Datei-Handler ========================

// wird aufgerufen in wlan.h: Website-Dateien für Captive Portal aus LittleFS bereitstellen
bool handleFileRead(String path)
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

  if (LittleFS.exists(path))
  {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

// ======================== Setup ========================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LED, OUTPUT);
  rgbLedWrite(PIN_LED, 0, 255, 0); // Rot (keine WLAN Verbindung)

  Wire.begin(I2C_SDA, I2C_SCL);
  setupDisplay();                   // in display.h
  displayText("Starte API Box..."); // in display.h

  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS Mount fehlgeschlagen!");
    displayText("LittleFS Fehler!"); // in display.h
  }

  Serial.println("\n-----------------------------\nInitialisiere Sensoren...");
  setupAlkohol();                         // in alkohol.h
  setupBewegung();                        // in bewegung.h
  setupCo2_Temperatur_Luftfeuchtigkeit(); // co2_temperatur_luftfeuchtigkeit.h
  setupDistanz();                         // in distanz.h
  setupGewicht();                         // in gewicht.h
  setupGPS();                             // in gps.h
  setupHelligkeit();                      // in helligkeit.h
  setupLage();                            // in lage.h
  setupLautstaerke();                     // in lautstaerke.h
  setupLuftdruck();                       // in luftdruck.h
  setupMagnet();                          // in magnet.h

  setupPortalButton(); // in wlan.h
  setupWLAN();         // in wlan.h
}

// ======================== Loop ========================

void loop()
{
  maintainWiFiConnection(); // in wlan.h
  if (millis() < prevTimestamp + intervall_ms)
    return;
  prevTimestamp = millis();

  float alkohol = getAlkohol(); // in alkohol.h
  Serial.printf("Alkohol: %.2f mg/L\n", alkohol);

  int bewegung = getBewegung(); // in bewegung.h
  Serial.printf("Bewegung: %d\n", bewegung);

  getCo2_Temperatur_Luftfeuchtigkeit(); // in co2_temperatur_luftfeuchtigkeit.h
  Serial.printf("CO2: %d ppm\n", co2);
  Serial.printf("Temperatur: %.2f °C\n", temperature);
  Serial.printf("Luftfeuchtigkeit: %.2f %%\n", humidity);

  int distanz = getDistanz(); // in distanz.h
  Serial.printf("Distanz: %d\n", distanz);

  float gewicht = getGewicht(); // in gewicht.h
  Serial.printf("Gewicht: %.2f g\n", gewicht);

  getGPS(); // in gps.h
  Serial.printf("Latitude: %.6f°\n", latitude);
  Serial.printf("Longitude: %.6f°\n", longitude);
  Serial.printf("Altitude: %.2f m\n", altitude);
  Serial.printf("Satelliten: %d\n", satellites);
  Serial.printf("timeString: %s\n", timeString.c_str());

  float helligkeit = getHelligkeit(); // in helligkeit.h
  Serial.printf("Helligkeit: %.2f lux\n", helligkeit);

  getLage(); // in lage.h
  Serial.printf("lage_x: %.2f\n", lage_x);
  Serial.printf("lage_y: %.2f\n", lage_y);

  float lautstaerke = getLautstaerke(); // in lautstaerke.h
  Serial.printf("Lautstärke: %.2f dB\n", lautstaerke);

  float luftdruck = getLuftdruck(); // in luftdruck.h
  Serial.printf("Luftdruck: %.2f hPa\n", luftdruck);

  int magnet = getMagnet(); // in magnet.h
  Serial.printf("Magnet: %.2f\n", magnet);

  String jsonResponse = "{\"alkohol\":" + String(alkohol) +
                        ",\"bewegung\":" + String(bewegung) +
                        ",\"co2\":" + String(co2) +
                        ",\"temperatur\":" + String(temperature) +
                        ",\"luftfeuchtigkeit\":" + String(humidity) +
                        ",\"distanz\":" + String(distanz) +
                        ",\"gewicht\":" + String(gewicht) +
                        ",\"latitude\":" + String(latitude) +
                        ",\"longitude\":" + String(longitude) +
                        ",\"altitude\":" + String(altitude) +
                        ",\"gps_num_satellites\":" + String(satellites) +
                        ",\"gps_time\":\"" + timeString +
                        ",\"helligkeit\":\"" + String(helligkeit) +
                        ",\"lage_x\":" + String(lage_x) +
                        ",\"lage_y\":" + String(lage_y) +
                        ",\"lautstaerke\":" + String(lautstaerke) +
                        ",\"luftdruck\":" + String(luftdruck) +
                        ",\"magnet\":" + String(magnet) +
                        "}";

  Serial.println(jsonResponse);
  // sende JSON String an set.php auf em Server.
}
