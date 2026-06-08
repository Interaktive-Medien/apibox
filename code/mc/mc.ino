/**********************************************************************************************
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
 *    - Adafruit SSD1306 + Adafruit GFX + Adafruit BusIO
 *    - Sensirion I2C SCD4x + Sensirion Core
 *    - BH1750 (Christopher Laws)
 *    - ICM20948_WE (Wolfgang Ewald)
 *    - HX711 Arduino Library (Bogdan Necula)
 *    - Adafruit BMP280 Library
 *    - Adafruit VL53L0X Library
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
 **********************************************************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <Wire.h>
#include "FS.h"
#include "LittleFS.h"

// ======================== JSON Helper ========================

String createJsonResponse(String wert, String einheit, String datentyp, String sensor) {
  return "{\"wert\":" + wert + ",\"einheit\":\"" + einheit + "\",\"datentyp\":\"" + datentyp + "\",\"sensor\":\"" + sensor + "\"}";
}

// ======================== Pin-Definitionen ========================

#define I2C_SDA 6
#define I2C_SCL 7
#define PIN_PIR 4
#define PIN_MQ3 5
#define PIN_MQ2 0
#define PIN_MAGNET 11
#define PIN_BUTTON_PORTAL 20
#define PIN_LED BUILTIN_LED

// ======================== Globale Objekte ========================

WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

const byte DNS_PORT = 53;
bool apMode = false;
bool wlanConnected = false;

// ======================== Sensor-Header einbinden ========================

#include "temperatur.h"        // SCD41 Temperatur
#include "luftfeuchtigkeit.h"  // SCD41 Luftfeuchtigkeit
#include "co2.h"               // SCD41 CO2
#include "bewegung.h"          // SR602 PIR
#include "lautstaerke.h"       // INMP441 Mikrofon
#include "magnet.h"            // GPS14-B Magnetsensor
#include "helligkeit.h"        // BH1750 Lichtsensor
#include "alkohol.h"           // MQ-3 Alkoholsensor
#include "lage.h"              // ICM-20948 9DOF
#include "gewicht.h"           // HX711 Waage
#include "rauch.h"             // MQ-2 Gas/Rauch
#include "luftdruck.h"         // BMP280 Luftdruck
#include "hoehe.h"             // BMP280 Hoehe
#include "distanz.h"           // VL53L0X Distanz
#include "display.h"           // SSD1306 OLED Display
#include "captive_portal.h"    // Captive Portal + WLAN Credentials

// ======================== LittleFS Datei-Handler ========================

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html"))      contentType = "text/html";
  else if (path.endsWith(".css"))  contentType = "text/css";
  else if (path.endsWith(".js"))   contentType = "application/javascript";
  else if (path.endsWith(".ico"))  contentType = "image/x-icon";
  else if (path.endsWith(".json")) contentType = "application/json";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

// ======================== API Endpunkte registrieren ========================

void setupAPIRoutes() {

  // Sensor-Endpunkte
  server.on("/temperatur", HTTP_GET, []() {
    server.send(200, "application/json", getTemperatur());         // temperatur.h
  });

  server.on("/luftfeuchtigkeit", HTTP_GET, []() {
    server.send(200, "application/json", getLuftfeuchtigkeit());   // luftfeuchtigkeit.h
  });

  server.on("/co2", HTTP_GET, []() {
    server.send(200, "application/json", getCO2());                // co2.h
  });

  server.on("/bewegung", HTTP_GET, []() {
    server.send(200, "application/json", getBewegung());           // bewegung.h
  });

  server.on("/lautstaerke", HTTP_GET, []() {
    server.send(200, "application/json", getLautstaerke());        // lautstaerke.h
  });

  server.on("/magnet", HTTP_GET, []() {
    server.send(200, "application/json", getMagnet());             // magnet.h
  });

  server.on("/helligkeit", HTTP_GET, []() {
    server.send(200, "application/json", getHelligkeit());         // helligkeit.h
  });

  server.on("/alkohol", HTTP_GET, []() {
    server.send(200, "application/json", getAlkohol());            // alkohol.h
  });

  server.on("/lage", HTTP_GET, []() {
    server.send(200, "application/json", getLage());               // lage.h
  });

  server.on("/gewicht", HTTP_GET, []() {
    server.send(200, "application/json", getGewicht());            // gewicht.h
  });

  server.on("/rauch", HTTP_GET, []() {
    server.send(200, "application/json", getRauch());              // rauch.h
  });

  server.on("/luftdruck", HTTP_GET, []() {
    server.send(200, "application/json", getLuftdruck());          // luftdruck.h
  });

  server.on("/hoehe", HTTP_GET, []() {
    server.send(200, "application/json", getHoehe());              // hoehe.h
  });

  server.on("/distanz", HTTP_GET, []() {
    server.send(200, "application/json", getDistanz());            // distanz.h
  });

  // Uebersicht aller Endpunkte
  server.on("/api", HTTP_GET, []() {
    String json = "{\"info\":\"API Box - ESP32-C6 Sensor API\",\"endpoints\":["
                  "\"/temperatur\",\"/luftfeuchtigkeit\",\"/co2\",\"/bewegung\","
                  "\"/lautstaerke\",\"/magnet\",\"/helligkeit\",\"/alkohol\","
                  "\"/lage\",\"/gewicht\",\"/rauch\",\"/luftdruck\","
                  "\"/hoehe\",\"/distanz\"]}";
    server.send(200, "application/json", json);
  });

  // LittleFS Fallback fuer statische Dateien (HTML/CSS/JS)
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: Not Found");
    }
  });
}

// ======================== Setup ========================

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Status-LED
  pinMode(PIN_LED, OUTPUT);
  rgbLedWrite(PIN_LED, 0, 255, 0);  // Rot = noch nicht verbunden

  // I2C Bus starten
  Wire.begin(I2C_SDA, I2C_SCL);

  // Display initialisieren
  setupDisplay();                                     // display.h
  displayText("Starte API Box...");

  // LittleFS initialisieren
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount fehlgeschlagen!");
    displayText("LittleFS Fehler!");
  }

  // Sensoren initialisieren
  setupTemperatur();                                  // temperatur.h
  setupBewegung();                                    // bewegung.h
  setupLautstaerke();                                 // lautstaerke.h
  setupMagnet();                                      // magnet.h
  setupHelligkeit();                                  // helligkeit.h
  setupAlkohol();                                     // alkohol.h
  setupLage();                                        // lage.h
  setupGewicht();                                     // gewicht.h
  setupRauch();                                       // rauch.h
  setupLuftdruck();                                   // luftdruck.h
  setupDistanz();                                     // distanz.h

  // Captive Portal Taster
  setupPortalButton();                                // captive_portal.h

  // WLAN Verbindung herstellen
  displayText("Suche bekanntes\nWLAN Netzwerk...");
  bool connected = connectToSavedWiFi();              // captive_portal.h

  if (connected) {
    apMode = false;
    wlanConnected = true;
    rgbLedWrite(PIN_LED, 255, 0, 0);  // Gruen = verbunden

    String info = "Verbunden mit\n" + WiFi.SSID() + "\n\nIP Adresse:\n" + WiFi.localIP().toString();
    displayText(info);
    Serial.printf("Verbunden: %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

    // API Endpunkte und Webserver starten
    setupAPIRoutes();
    server.begin();
    Serial.println("HTTP Webserver gestartet.");

  } else {
    startCaptivePortal();                             // captive_portal.h
    apMode = true;
    wlanConnected = false;
    rgbLedWrite(PIN_LED, 0, 255, 0);  // Rot = AP Modus
  }
}

// ======================== Loop ========================

void loop() {
  // DNS im AP-Modus
  if (apMode) {
    dnsServer.processNextRequest();
  }

  // Webserver-Anfragen verarbeiten
  server.handleClient();

  // Taster pruefen: Captive Portal manuell starten
  if (checkPortalButton()) {                          // captive_portal.h
    Serial.println("Taster gedrueckt -> Captive Portal starten");
    displayText("Captive Portal\nwird gestartet...");
    startCaptivePortal();                             // captive_portal.h
    apMode = true;
    wlanConnected = false;
    rgbLedWrite(PIN_LED, 0, 255, 0);  // Rot
  }

  // Reconnect im STA-Modus bei Verbindungsverlust
  if (!apMode && WiFi.status() != WL_CONNECTED) {
    if (wlanConnected) {
      Serial.println("WLAN Verbindung verloren, reconnect...");
      displayText("WLAN verloren...\nReconnect...");
      rgbLedWrite(PIN_LED, 0, 255, 0);  // Rot
      wlanConnected = false;
    }
    WiFi.reconnect();
    delay(5000);
    if (WiFi.status() == WL_CONNECTED) {
      wlanConnected = true;
      rgbLedWrite(PIN_LED, 255, 0, 0);  // Gruen
      String info = "Verbunden mit\n" + WiFi.SSID() + "\n\nIP Adresse:\n" + WiFi.localIP().toString();
      displayText(info);
    }
  }
}
