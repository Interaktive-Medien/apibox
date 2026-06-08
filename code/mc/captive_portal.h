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
 *    "mode"  = "home" | "school"
 *    "ssid"  = Netzwerkname
 *    "pass"  = Passwort
 *    "user"  = Username (nur Schulnetzwerk / WPA2-Enterprise)
 *
 *  Taster:  GPIO20 (INPUT_PULLDOWN). Orientiert an 00_Button.ino.
 *  Captive Portal orientiert an 14c_mit_Webserver_captive_portal.ino.
 **********************************************************************************************/

#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "esp_eap_client.h"   // WPA2-Enterprise API (Schulnetzwerk)

bool portalRoutesRegistered = false;

// ---------------------------------------------------------------------------
// Taster (GPIO20) zum manuellen Start des Captive Portals
// ---------------------------------------------------------------------------
void setupPortalButton() {
  pinMode(PIN_BUTTON_PORTAL, INPUT_PULLDOWN);   // PIN_BUTTON_PORTAL = 20 (aus mc.ino)
}

bool checkPortalButton() {
  static int prevState = LOW;
  int state = digitalRead(PIN_BUTTON_PORTAL);
  bool pressed = (state == HIGH && prevState == LOW);   // steigende Flanke
  prevState = state;
  return pressed;
}

// ---------------------------------------------------------------------------
// Mit gespeichertem WLAN verbinden (Szenario 1)
// Rueckgabe: true = verbunden, false = kein Netzwerk gefunden
// ---------------------------------------------------------------------------
bool connectToSavedWiFi() {
  preferences.begin("wifi", true);            // read-only (global aus mc.ino)
  String mode = preferences.getString("mode", "");
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  String user = preferences.getString("user", "");
  preferences.end();

  if (ssid == "") {
    Serial.println("Keine gespeicherten WLAN-Credentials.");
    return false;
  }

  Serial.printf("Verbinde mit gespeichertem WLAN: %s (Modus: %s)\n", ssid.c_str(), mode.c_str());

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  if (mode == "school") {
    // WPA2-Enterprise (z.B. eduroam)
    esp_eap_client_set_identity((uint8_t *)user.c_str(), user.length());
    esp_eap_client_set_username((uint8_t *)user.c_str(), user.length());
    esp_eap_client_set_password((uint8_t *)pass.c_str(), pass.length());
    esp_eap_client_set_disable_time_check(true);
    esp_wifi_sta_enterprise_enable();
    WiFi.begin(ssid.c_str());
  } else {
    // Heimnetzwerk (WPA2-Personal)
    WiFi.begin(ssid.c_str(), pass.c_str());
  }

  // max. 10 Versuche / ca. 10 Sekunden
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  return (WiFi.status() == WL_CONNECTED);
}

// ---------------------------------------------------------------------------
// HTML der Setup-Seite (modern gestyltes Formular mit zwei Reitern)
// ---------------------------------------------------------------------------
String portalHtml() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>API Box — WLAN Setup</title>
<style>
  * { box-sizing: border-box; }
  body { margin:0; font-family: -apple-system, Segoe UI, Roboto, sans-serif;
         background: linear-gradient(135deg,#1e3c72,#2a5298); color:#222;
         min-height:100vh; display:flex; align-items:center; justify-content:center; }
  .card { background:#fff; width:92%; max-width:420px; border-radius:18px;
          box-shadow:0 20px 50px rgba(0,0,0,.3); padding:28px; }
  h1 { margin:0 0 4px; font-size:22px; }
  p.sub { margin:0 0 20px; color:#666; font-size:14px; }
  .tabs { display:flex; background:#eef1f6; border-radius:12px; padding:4px; margin-bottom:20px; }
  .tab { flex:1; text-align:center; padding:10px; border-radius:9px; cursor:pointer;
         font-weight:600; font-size:14px; color:#555; user-select:none; }
  .tab.active { background:#2a5298; color:#fff; }
  label { display:block; font-size:13px; font-weight:600; margin:14px 0 6px; }
  input { width:100%; padding:12px; border:1px solid #ccd2dd; border-radius:10px; font-size:15px; }
  input:focus { outline:none; border-color:#2a5298; }
  button { width:100%; margin-top:22px; padding:14px; border:0; border-radius:10px;
           background:#2a5298; color:#fff; font-size:16px; font-weight:700; cursor:pointer; }
  button:active { transform:scale(.99); }
  .form { display:none; }
  .form.active { display:block; }
  .hint { font-size:12px; color:#888; margin-top:14px; text-align:center; }
</style>
</head>
<body>
  <div class="card">
    <h1>API Box</h1>
    <p class="sub">WLAN-Zugangsdaten hinterlegen</p>

    <div class="tabs">
      <div class="tab active" id="tab-home"   onclick="showTab('home')">Heimnetzwerk</div>
      <div class="tab"        id="tab-school" onclick="showTab('school')">Schulnetzwerk</div>
    </div>

    <!-- Heimnetzwerk -->
    <form class="form active" id="form-home" action="/save" method="POST">
      <input type="hidden" name="mode" value="home">
      <label>SSID</label>
      <input type="text" name="ssid" placeholder="z.B. FritzBox-1234" required>
      <label>Passwort</label>
      <input type="password" name="pass" placeholder="WLAN-Passwort">
      <button type="submit">Speichern &amp; Verbinden</button>
    </form>

    <!-- Schulnetzwerk (WPA2-Enterprise) -->
    <form class="form" id="form-school" action="/save" method="POST">
      <input type="hidden" name="mode" value="school">
      <label>Netzwerkname</label>
      <input type="text" name="ssid" placeholder="z.B. eduroam" required>
      <label>Username</label>
      <input type="text" name="user" placeholder="z.B. me@fhgr.ch" required>
      <label>Passwort</label>
      <input type="password" name="pass" placeholder="Passwort">
      <button type="submit">Speichern &amp; Verbinden</button>
    </form>

    <p class="hint">Nach dem Speichern startet die API Box neu.</p>
  </div>

<script>
function showTab(which){
  document.getElementById('tab-home').classList.toggle('active', which==='home');
  document.getElementById('tab-school').classList.toggle('active', which==='school');
  document.getElementById('form-home').classList.toggle('active', which==='home');
  document.getElementById('form-school').classList.toggle('active', which==='school');
}
</script>
</body>
</html>
)rawliteral";
  return page;
}

// ---------------------------------------------------------------------------
// Webserver-Handler fuer das Captive Portal
// ---------------------------------------------------------------------------
void handlePortalRoot() {
  server.send(200, "text/html", portalHtml());
}

void handlePortalSave() {
  String mode = server.hasArg("mode") ? server.arg("mode") : "home";
  String ssid = server.hasArg("ssid") ? server.arg("ssid") : "";
  String pass = server.hasArg("pass") ? server.arg("pass") : "";
  String user = server.hasArg("user") ? server.arg("user") : "";

  // Persistent speichern
  preferences.begin("wifi", false);
  preferences.putString("mode", mode);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.putString("user", user);
  preferences.end();

  String resp = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  resp += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  resp += "<style>body{font-family:sans-serif;background:#2a5298;color:#fff;";
  resp += "display:flex;align-items:center;justify-content:center;height:100vh;text-align:center}</style>";
  resp += "</head><body><div><h2>Gespeichert!</h2>";
  resp += "<p>SSID: " + ssid + "</p><p>Die API Box startet neu und verbindet sich...</p></div></body></html>";
  server.send(200, "text/html", resp);

  Serial.println("Credentials gespeichert. Neustart in 2s...");
  delay(2000);
  ESP.restart();   // sauberer Neustart -> connectToSavedWiFi() beim Boot
}

// ---------------------------------------------------------------------------
// Captive Portal starten (Szenario 2)
// ---------------------------------------------------------------------------
void startCaptivePortal() {
  Serial.println("Starte Access Point 'apibox'...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("apibox");                       // offen, ohne Passwort

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP-IP: ");
  Serial.println(apIP);

  // DNS: alle Domains -> ESP (Captive Portal)
  dnsServer.start(DNS_PORT, "*", apIP);

  if (!portalRoutesRegistered) {
    server.on("/", HTTP_GET, handlePortalRoot);
    server.on("/save", HTTP_POST, handlePortalSave);
    server.onNotFound(handlePortalRoot);       // alles auf Setup-Seite leiten
    portalRoutesRegistered = true;
  }
  server.begin();

  // Anzeige auf dem Display
  displayText("Verbinde dich mit\ndem Netzwerk\n'apibox' und\nhinterlege deine\nWLAN Credentials.");   // display.h
  Serial.println("Captive Portal aktiv (apibox).");
}

#endif
