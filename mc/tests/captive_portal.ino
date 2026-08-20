/**********************************************************************************************
*  captive_portal.ino  (Test/Demo)
*  Access Point + Captive Portal: AP ohne Passwort, DNS leitet alle Domains auf den ESP.
*  Setup-Webseite zur Eingabe von SSID/Passwort, persistent in Preferences gespeichert.
*  Original-Demo 14c_mit_Webserver_captive_portal.ino — Referenz fuer mc/captive_portal.h.
*  Libraries: WiFi, WebServer, Preferences, DNSServer (alle eingebaut).
***********************************************************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>

Preferences preferences;

String targetSSID = "";
String targetPassword = "";
bool apMode = false;

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

String htmlPage();
void handleSave();
void handleRoot();

void setup() {
  Serial.begin(115200);
  delay(1000);

  preferences.begin("wifi", true);
  targetSSID = preferences.getString("ssid", "");
  targetPassword = preferences.getString("password", "");
  preferences.end();

  if (targetSSID != "") {
    Serial.print("Versuche Verbindung mit gespeichertem WLAN: ");
    Serial.println(targetSSID);
    WiFi.begin(targetSSID.c_str(), targetPassword.c_str());
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Verbunden: SSID: %s, IP: %s\n", targetSSID.c_str(), WiFi.localIP().toString().c_str());
    apMode = false;
  } else {
    Serial.println("Keine Verbindung, starte Access Point...");
    WiFi.softAP("ESP32-Setup");
    IPAddress apIP = WiFi.softAPIP();
    Serial.print("AP-IP: ");
    Serial.println(apIP);
    dnsServer.start(DNS_PORT, "*", apIP);
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.onNotFound(handleRoot);
    server.begin();
    apMode = true;
  }
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}

String htmlPage() {
  String page = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  page += "<title>ESP32-C6 Setup</title></head><body>";
  page += "<h2>WLAN Einstellungen</h2>";
  page += "<form action='/save' method='POST'>";
  page += "SSID: <input type='text' name='ssid' value='" + targetSSID + "'><br><br>";
  page += "Passwort: <input type='password' name='password'><br><br>";
  page += "<input type='submit' value='Speichern & Verbinden'>";
  page += "</form></body></html>";
  return page;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSave() {
  if (server.method() == HTTP_POST) {
    if (server.hasArg("ssid")) targetSSID = server.arg("ssid");
    if (server.hasArg("password")) targetPassword = server.arg("password");

    preferences.begin("wifi", false);
    preferences.putString("ssid", targetSSID);
    preferences.putString("password", targetPassword);
    preferences.end();

    server.send(200, "text/html", "<h2>Gespeichert! ESP verbindet sich...</h2>");
    delay(1000);
    WiFi.begin(targetSSID.c_str(), targetPassword.c_str());
  }
}
