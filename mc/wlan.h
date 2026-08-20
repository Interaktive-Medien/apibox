/**********************************************************************************************
 *  wlan.h
 *  WLAN-Verbindung + Captive Portal + Reconnect-Logik + persistente Credentials
 *  Fasst alle WLAN-Funktionen zusammen.
 **********************************************************************************************/

#ifndef WLAN_H
#define WLAN_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "esp_eap_client.h" // WPA2-Enterprise API

// ---------------------------------------------------------------------------
// Globale WLAN-Objekte und Variablen
// ---------------------------------------------------------------------------
WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

const byte DNS_PORT = 53;
bool apMode = false;
bool wlanConnected = false;
bool portalRoutesRegistered = false;

#define PIN_LED BUILTIN_LED

// Forward Declarations für Funktionen aus anderen Headern/Dateien
void displayText(String text);    // in display.h
bool handleFileRead(String path); // in mc.ino
void setupAPIRoutes();            // in mc.ino

// ---------------------------------------------------------------------------
// Taster (GPIO20) zum manuellen Start des Captive Portals
// ---------------------------------------------------------------------------
// aufgerufen in mc.ino
void setupPortalButton()
{
  pinMode(PIN_BUTTON_PORTAL, INPUT_PULLDOWN);
}

bool checkPortalButton()
{
  static int prevState = LOW;
  int state = digitalRead(PIN_BUTTON_PORTAL);
  bool pressed = (state == HIGH && prevState == LOW);
  prevState = state;
  return pressed;
}

// ---------------------------------------------------------------------------
// Captive Portal Handler & Funktionen
// ---------------------------------------------------------------------------
void handlePortalRoot()
{
  if (LittleFS.exists("/index.html"))
  {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  }
  else
  {
    server.send(404, "text/plain", "Setup-Seite nicht in LittleFS gefunden.");
  }
}

void handlePortalSave()
{
  String mode = server.hasArg("mode") ? server.arg("mode") : "home";
  String ssid = server.hasArg("ssid_privat") ? server.arg("ssid_privat") : (server.hasArg("ssid_schule") ? server.arg("ssid_schule") : "");
  String pass = server.hasArg("pass_privat") ? server.arg("pass_privat") : (server.hasArg("pass_schule") ? server.arg("pass_schule") : "");
  String user = server.hasArg("user_schule") ? server.arg("user_schule") : "";

  // Falls generische Namen verwendet werden (Fallback)
  if (ssid == "" && server.hasArg("ssid"))
    ssid = server.arg("ssid");
  if (pass == "" && server.hasArg("pass"))
    pass = server.arg("pass");
  if (user == "" && server.hasArg("user"))
    user = server.arg("user");

  preferences.begin("wifi", false);
  preferences.putString("mode", mode);

  if (mode == "school")
  {
    preferences.putString("ssid_schule", ssid);
    preferences.putString("user_schule", user);
    preferences.putString("pass_schule", pass);
  }
  else
  {
    preferences.putString("ssid_privat", ssid);
    preferences.putString("pass_privat", pass);
  }
  preferences.end();

  String resp = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  resp += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  resp += "<style>body{font-family:sans-serif;background:#2a5298;color:#fff;";
  resp += "display:flex;align-items:center;justify-content:center;height:100vh;text-align:center}</style>";
  resp += "</head><body><div><h2>Gespeichert!</h2>";
  resp += "<p>SSID: " + ssid + "</p>";
  resp += "<p>Die API Box startet neu und verbindet sich...</p></div></body></html>";
  server.send(200, "text/html", resp);

  Serial.println("Credentials gespeichert. Neustart in 2s...");
  delay(2000);
  ESP.restart();
}

void startCaptivePortal()
{
  Serial.println("\n-----------------------------\nStarte captive WLAN...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("apibox");

  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  if (!portalRoutesRegistered)
  {
    server.on("/", HTTP_GET, handlePortalRoot);
    server.on("/save", HTTP_POST, handlePortalSave);
    server.onNotFound([]()
                      {
      if (!handleFileRead(server.uri())) { // in mc.ino
        handlePortalRoot();
      } });
    portalRoutesRegistered = true;
  }
  server.begin();

  String info = "AP Modus\nIP Adresse:\n" + WiFi.softAPIP().toString() + "\nDNS Adresse:\n" + WiFi.softAPIP().toString();
  displayText(info); // in display.h
  Serial.printf("AP Modus gestartet. AP IP: %s, DNS: %s\n", WiFi.softAPIP().toString().c_str(), WiFi.softAPIP().toString().c_str());
}

// aufgerufen in mc.ino
void handlePortalButton()
{
  if (checkPortalButton())
  {
    Serial.println("Taster gedrueckt -> Captive Portal starten");
    displayText("Captive Portal\nwird gestartet..."); // in display.h
    startCaptivePortal();
    apMode = true;
    wlanConnected = false;
    rgbLedWrite(PIN_LED, 0, 255, 0); // Rot / AP-Modus
  }
}

// ---------------------------------------------------------------------------
// WLAN Verbindung & Reconnect Logik
// ---------------------------------------------------------------------------
bool connectToSavedWiFi()
{
  preferences.begin("wifi", true);
  String mode = preferences.getString("mode", "home");
  String ssid_p = preferences.getString("ssid_privat", "");
  String pass_p = preferences.getString("pass_privat", "");
  String ssid_s = preferences.getString("ssid_schule", "");
  String user_s = preferences.getString("user_schule", "");
  String pass_s = preferences.getString("pass_schule", "");
  preferences.end();

  String ssid, pass, user;
  if (mode == "school")
  {
    ssid = ssid_s;
    user = user_s;
    pass = pass_s;
  }
  else
  {
    ssid = ssid_p;
    pass = pass_p;
  }

  if (ssid == "")
    return false;

  Serial.printf("Verbinde mit WLAN: %s (%s)\n", ssid.c_str(), mode.c_str());
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  if (mode == "school")
  {
    esp_eap_client_set_identity((uint8_t *)user.c_str(), user.length());
    esp_eap_client_set_username((uint8_t *)user.c_str(), user.length());
    esp_eap_client_set_password((uint8_t *)pass.c_str(), pass.length());
    esp_eap_client_set_disable_time_check(true);
    esp_wifi_sta_enterprise_enable();
    WiFi.begin(ssid.c_str());
  }
  else
  {
    WiFi.begin(ssid.c_str(), pass.c_str());
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  return (WiFi.status() == WL_CONNECTED);
}

// aufgerufen in mc.ino
void maintainWiFiConnection()
{
  // DNS-Anfragen abarbeiten, nur wenn wir im Access Point Modus sind.
  if (apMode)
  {
    dnsServer.processNextRequest();
  }

  // Webserver-Anfragen verarbeiten
  server.handleClient();

  // Taster pruefen: Captive Portal manuell starten
  handlePortalButton();

  // Reconnect im STA-Modus bei Verbindungsverlust
  if (!apMode && WiFi.status() != WL_CONNECTED)
  {
    if (wlanConnected)
    {
      Serial.println("WLAN Verbindung verloren, reconnect...");
      displayText("WLAN verloren...\nReconnect..."); // in display.h
      rgbLedWrite(PIN_LED, 0, 255, 0);               // Rot
      wlanConnected = false;
    }
    WiFi.reconnect();
    delay(5000);
    if (WiFi.status() == WL_CONNECTED)
    {
      wlanConnected = true;
      rgbLedWrite(PIN_LED, 255, 0, 0); // Gruen
      String info = "Verbunden mit\n" + WiFi.SSID() + "\n\nIP Adresse:\n" + WiFi.localIP().toString();
      displayText(info); // in display.h
    }
  }
}

// aufgerufen in mc.ino
void setupWLAN()
{
  displayText("Suche bekanntes\nWLAN Netzwerk..."); // in display.h
  Serial.println("\n-----------------------------\nSuche bekanntes WLAN Netzwerk...");

  if (connectToSavedWiFi())
  {
    apMode = false;
    wlanConnected = true;
    rgbLedWrite(PIN_LED, 255, 0, 0); // Gruen = verbunden

    String info = "Verbunden mit\n" + WiFi.SSID() + "\n\nIP Adresse:\n" + WiFi.localIP().toString();
    displayText(info); // in display.h
    Serial.printf("Verbunden: %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

    // API Endpunkte und Webserver starten
    setupAPIRoutes(); // in mc.ino: roting der API-Endpunkte, z.B. http://[IP]/temperatur -> getTemperatur() in temperatur.h
    server.begin();
    Serial.println("HTTP Webserver gestartet.");
  }
  else
  {
    startCaptivePortal();
    apMode = true;
    wlanConnected = false;
    rgbLedWrite(PIN_LED, 0, 255, 0); // Rot = AP Modus
  }
}

#endif
