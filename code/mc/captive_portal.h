/**********************************************************************************************
 *  captive_portal.h
 *  WLAN-Verbindung + Captive Portal + persistente Credentials (Preferences)
 *  Libraries: WiFi, WebServer, DNSServer, Preferences (alle eingebaut),
 *             esp_eap_client.h (fuer WPA2-Enterprise / Schulnetzwerk)
 *
 *  Funktionen:
 *    connectToSavedWiFi()  -> laedt gespeicherte Credentials, verbindet (max ~10s)
 *    startCaptivePortal()  -> startet AP "apibox" + DNS + Setup-Webseite
 *    setupPortalButton()   -> Taster GPIO20 (INPUT_PULLDOWN) initialisieren
 *    checkPortalButton()   -> true, wenn Taster gedrueckt (manueller Portal-Start)
 *
 *  Preferences-Namespace "wifi":
 *    "mode"          = "home" | "school"
 *    "ssid_privat"   = Netzwerkname (Home)
 *    "pass_privat"   = Passwort (Home)
 *    "ssid_schule"   = Netzwerkname (School)
 *    "user_schule"   = Username (School)
 *    "pass_schule"   = Passwort (School)
 *
 *  Beispiele:
 *    ssid_privat = tinkergarden
 *    ssid_schule = eduroam
 *    user_schule = fiesjan@fhgr.ch
 *    pass_privat = strenggeheim.
 *
 *  Taster:  GPIO20 (INPUT_PULLDOWN). Orientiert an 00_Button.ino.
 *  Captive Portal orientiert an 14c_mit_Webserver_captive_portal.ino.
 **********************************************************************************************/

#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "esp_eap_client.h" // WPA2-Enterprise API (Schulnetzwerk)

bool portalRoutesRegistered = false;

// ---------------------------------------------------------------------------
// Taster (GPIO20) zum manuellen Start des Captive Portals
// ---------------------------------------------------------------------------
void setupPortalButton()
{
  pinMode(PIN_BUTTON_PORTAL, INPUT_PULLDOWN); // PIN_BUTTON_PORTAL = 20 (aus mc.ino)
}

bool checkPortalButton()
{
  static int prevState = LOW;
  int state = digitalRead(PIN_BUTTON_PORTAL);
  bool pressed = (state == HIGH && prevState == LOW); // steigende Flanke
  prevState = state;
  return pressed;
}

// ---------------------------------------------------------------------------
// Mit gespeichertem WLAN verbinden (Szenario 1)
// Rueckgabe: true = verbunden, false = kein Netzwerk gefunden
// ---------------------------------------------------------------------------
bool connectToSavedWiFi()
{
  preferences.begin("wifi", true); // read-only (global aus mc.ino)
  String mode = preferences.getString("mode", "home");
  String ssid_privat = preferences.getString("ssid_privat", "");
  String pass_privat = preferences.getString("pass_privat", "");
  String ssid_schule = preferences.getString("ssid_schule", "");
  String user_schule = preferences.getString("user_schule", "");
  String pass_schule = preferences.getString("pass_schule", "");

  // Serial.println("Gelesene WLAN-Informationen:");
  // Serial.print("mode: ");
  // Serial.println(mode);
  // Serial.print("ssid_privat: ");
  // Serial.println(ssid_privat);
  // Serial.print("pass_privat: ");
  // Serial.println(pass_privat);
  // Serial.print("ssid_schule: ");
  // Serial.println(ssid_schule);
  // Serial.print("user_schule: ");
  // Serial.println(user_schule);
  // Serial.print("pass_schule: ");
  // Serial.println(pass_schule);

  preferences.end();

  String ssid_to_connect, pass_to_connect, user_to_connect;
  if (mode == "school")
  {
    ssid_to_connect = ssid_schule;
    user_to_connect = user_schule;
    pass_to_connect = pass_schule;
  }
  else
  {
    ssid_to_connect = ssid_privat;
    pass_to_connect = pass_privat;
  }

  if (ssid_to_connect == "")
  {
    Serial.println("Keine gespeicherten WLAN-Credentials.");
    return false;
  }

  Serial.printf("\nVerbinde mit gespeichertem WLAN: %s (Modus: %s)\n", ssid_to_connect.c_str(), mode.c_str());

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  if (mode == "school")
  {
    // WPA2-Enterprise (z.B. eduroam)
    esp_eap_client_set_identity((uint8_t *)user_to_connect.c_str(), user_to_connect.length());
    esp_eap_client_set_username((uint8_t *)user_to_connect.c_str(), user_to_connect.length());
    esp_eap_client_set_password((uint8_t *)pass_to_connect.c_str(), pass_to_connect.length());
    esp_eap_client_set_disable_time_check(true);
    esp_wifi_sta_enterprise_enable();
    WiFi.begin(ssid_to_connect.c_str());
  }
  else
  {
    // Heimnetzwerk (WPA2-Personal)
    WiFi.begin(ssid_to_connect.c_str(), pass_to_connect.c_str());
  }

  // max. 10 Versuche / ca. 10 Sekunden
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

// ---------------------------------------------------------------------------
// Vorwaertsdeklaration (in mc.ino definiert)
// ---------------------------------------------------------------------------
bool handleFileRead(String path);

// ---------------------------------------------------------------------------
// Webserver-Handler fuer das Captive Portal
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
  String ssid_privat = server.hasArg("ssid_privat") ? server.arg("ssid_privat") : "";
  String pass_privat = server.hasArg("pass_privat") ? server.arg("pass_privat") : "";
  String ssid_schule = server.hasArg("ssid_schule") ? server.arg("ssid_schule") : "";
  String user_schule = server.hasArg("user_schule") ? server.arg("user_schule") : "";
  String pass_schule = server.hasArg("pass_schule") ? server.arg("pass_schule") : "";

  // Persistent speichern
  preferences.begin("wifi", false);
  preferences.putString("mode", mode);

  if (mode == "school")
  {
    preferences.putString("ssid_schule", ssid_schule);
    preferences.putString("user_schule", user_schule);
    preferences.putString("pass_schule", pass_schule);
  }
  else if (mode == "home")
  {
    preferences.putString("ssid_privat", ssid_privat);
    preferences.putString("pass_privat", pass_privat);
  }
  preferences.end();

  String resp = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  resp += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  resp += "<style>body{font-family:sans-serif;background:#2a5298;color:#fff;";
  resp += "display:flex;align-items:center;justify-content:center;height:100vh;text-align:center}</style>";
  resp += "</head><body><div><h2>Gespeichert!</h2>";
  resp += "<p>SSID: " + (mode == "school" ? ssid_schule : ssid_privat) + "</p>";
  resp += "<p>Die API Box startet neu und verbindet sich...</p></div></body></html>";
  server.send(200, "text/html", resp);

  Serial.println("Credentials gespeichert. Neustart in 2s...");
  delay(2000);
  ESP.restart(); // sauberer Neustart -> connectToSavedWiFi() beim Boot
}

// ---------------------------------------------------------------------------
// Captive Portal starten (Szenario 2)
// ---------------------------------------------------------------------------
void startCaptivePortal()
{
  Serial.println("Starte Access Point 'apibox'...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("apibox"); // offen, ohne Passwort

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP-IP: ");
  Serial.println(apIP);

  // DNS: alle Domains -> ESP (Captive Portal)
  dnsServer.start(DNS_PORT, "*", apIP);

  if (!portalRoutesRegistered)
  {
    server.on("/", HTTP_GET, handlePortalRoot);
    server.on("/save", HTTP_POST, handlePortalSave);

    // onNotFound: Erst versuchen Datei zu lesen (CSS/JS), sonst Setup-Seite
    server.onNotFound([]()
                      {
      if (!handleFileRead(server.uri())) {
        handlePortalRoot();
      } });
    portalRoutesRegistered = true;
  }
  server.begin();

  // Anzeige auf dem Display
  displayText("Verbinde dich mit\ndem Netzwerk\n'apibox' und\nhinterlege deine\nWLAN Credentials."); // display.h
  Serial.println("Captive Portal aktiv (apibox).");
}

#endif
