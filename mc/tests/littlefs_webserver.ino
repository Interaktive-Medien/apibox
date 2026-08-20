/******************************************************************************************************
 * littlefs_webserver.ino  (Test/Demo)
 * ESP32-C6 als Webserver: liefert HTML/CSS/JS direkt aus dem LittleFS-Dateisystem aus.
 * Original-Demo (Kap. 11, webserver_html_css_js) — Referenz fuer den Webserver in mc.ino.
 * Libraries: WiFi, WebServer, FS, LittleFS (alle eingebaut).
 *
 * Upload der Dateien: lege sie in "data/" ab und nutze
 * CMD + SHIFT + P -> "Upload LittleFS to Pico/ESP8266/ESP32".
 ******************************************************************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include "FS.h"
#include "LittleFS.h"

const char* ssid = "MeinNetzwerk";
const char* pass = "MeinPasswort";

WebServer server(80);

// Datei aus LittleFS an den Browser streamen
bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";

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
  delay(1000);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount fehlgeschlagen!");
    return;
  }

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());

  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: File Not Found");
    }
  });

  server.begin();
  Serial.println("Webserver gestartet.");
}

void loop() {
  server.handleClient();
}
