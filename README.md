# API Box — ESP32-C6 Sensor-API

**Modul:** Interaktive Medien 3 an der Fachhochschule Graubünden
**Studiengang:** Multimedia Production · 3. Semester
**Board:** Waveshare ESP32-C6-N8

> Eine wasserdichte IoT-Applikation auf dem ESP32-C6, die als **HTTP-API-Endpunkt**
> fungiert. Angeschlossene Sensoren werden in Echtzeit per HTTP GET abgefragt und als
> **JSON** zurückgegeben — bereit zum Einsammeln und Auswerten über einen längeren Zeitraum.

*[Platz für Bilder / GIFs des fertigen Aufbaus]*

---

## Kurzbeschreibung des Projekts

Die **API Box** ist ein kleiner Webserver auf dem ESP32-C6. Sie verbindet sich mit einem
WLAN und stellt für jeden angeschlossenen Sensor einen eigenen API-Endpunkt bereit.
Ein beliebiger Netzwerkteilnehmer (Browser, JavaScript `fetch()`, PHP `cURL`) ruft einen
Endpunkt auf und erhält den aktuellen Messwert als JSON-String:

```
GET http://192.168.0.42/temperatur

{
  "wert": 22.7,
  "einheit": "°C",
  "datentyp": "float",
  "sensor": "SCD41"
}
```

Werte werden **punktuell** abgefragt (kein dauerhaftes Logging auf dem ESP) — gespeichert
und ausgewertet wird auf der Client-/Serverseite.

Findet der ESP kein bekanntes Netzwerk, startet er ein **Captive Portal** (Access Point
`apibox`) mit einem modernen Formular zur Eingabe der WLAN-Zugangsdaten. Ein **SSD1306
OLED** zeigt jederzeit den Status (Suche / verbunden + IP / Portal-Hinweis).

---

## Setup

### Was benötige ich an Infrastruktur?

* **Waveshare ESP32-C6-N8** + USB-C-Kabel
* **Arduino IDE** (2.x) mit installiertem **ESP32 Arduino Core 3.3.x** (Boardverwalter)
* Das Plugin **[arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)**
  (zum Hochladen der Website-Dateien)
* Breadboard, Jumperkabel, die unten gelisteten Sensoren
* Ein WLAN-Netzwerk (Heim **oder** Schule/eduroam)

### Benötigte Arduino-Libraries

Über **Library Manager** installieren:

| Library | Zweck |
|---|---|
| `Arduino_JSON` (Arduino) | JSON-Antworten erzeugen |
| `Adafruit SSD1306` (+ GFX, + BusIO) | OLED-Display |
| `Sensirion I2C SCD4x` (+ Sensirion Core) | SCD41: Temperatur, Luftfeuchtigkeit, CO2 |
| `BH1750` (Christopher Laws) | Helligkeit |
| `ICM20948_WE` (Wolfgang Ewald) | Lage / 9DOF |
| `HX711 Arduino Library` (Bogdan Necula) | Gewicht |
| `Adafruit BMP280 Library` (+ Adafruit Unified Sensor) | Luftdruck + Höhe |
| `Adafruit VL53L0X` | Distanz |

`WiFi`, `WebServer`, `DNSServer`, `Preferences`, `Wire`, `LittleFS`, `driver/i2s.h`
sind im ESP32-Core bereits enthalten.

### Komponentenplan

```
                         ┌─────────────────────────────┐
        WLAN  ◀────────▶ │   Waveshare ESP32-C6-N8     │
   (Heim / eduroam)      │   ── mc.ino (Webserver) ──  │
                         └──────────────┬──────────────┘
                                        │
         ┌──────────────────────────────┼───────────────────────────────┐
         │ I2C-Bus (GPIO6 SDA / GPIO7 SCL)                               │
         │   ├─ SSD1306 OLED      → display.h        (Statusanzeige)     │
         │   ├─ SCD41             → temperatur.h / luftfeuchtigkeit.h /  │
         │   │                      co2.h            (/temperatur …)     │
         │   ├─ BH1750            → helligkeit.h     (/helligkeit)       │
         │   ├─ ICM-20948         → lage.h           (/lage)            │
         │   ├─ BMP280            → luftdruck.h / hoehe.h (/luftdruck…)  │
         │   └─ VL53L0X           → distanz.h        (/distanz)          │
         │                                                               │
         │ Digital / Analog / I2S                                        │
         │   ├─ SR602  PIR  GPIO4        → bewegung.h    (/bewegung)     │
         │   ├─ GPS14-B     GPIO11       → magnet.h      (/magnet)       │
         │   ├─ MQ-3        GPIO5 (ADC)  → alkohol.h     (/alkohol)      │
         │   ├─ MQ-2        GPIO0 (ADC)  → rauch.h       (/rauch)        │
         │   ├─ INMP441 I2S GPIO2/13/23  → lautstaerke.h (/lautstaerke)  │
         │   ├─ HX711       GPIO21/22    → gewicht.h     (/gewicht)      │
         │   └─ Taster      GPIO20       → captive_portal.h (Portal)     │
         │                                                               │
         │ Webseite (LittleFS): data/index.html + css + js               │
         └───────────────────────────────────────────────────────────────┘
                                        │
        Client: fetch() (JS)  ◀────────┴────────▶  Server: cURL (PHP)
```

### Verkabelung / Pinbelegung

| ESP32-C6 Pin | Komponente | Hinweis |
|---|---|---|
| **GPIO6 (SDA) / GPIO7 (SCL)** | I2C-Bus: SSD1306, SCD41, BH1750, ICM-20948, BMP280, VL53L0X | gemeinsamer Bus |
| GPIO4 | SR602 PIR (Bewegung) | Input |
| GPIO5 | MQ-3 (Alkohol) | Analog/ADC1, VCC **5V** |
| GPIO0 | MQ-2 (Rauch) | Analog/ADC1, VCC **5V** |
| GPIO11 | GPS14-B (Magnet) | INPUT_PULLUP |
| GPIO20 | Taster (Captive Portal) | INPUT_PULLDOWN |
| GPIO2 / GPIO13 / GPIO23 | INMP441 SCK / SD / WS | I2S, L/R → GND |
| **GPIO21 / GPIO22** | HX711 DT / SCK (Waage) | **siehe Warnung unten** |
| 3.3V / 5V / GND | Versorgung | MQ-Sensoren brauchen 5V |

> ⚠️ **Wichtig — HX711 NICHT auf GPIO6/7!**
> Im ursprünglichen Steckplan lag der HX711 (Waage) auf GPIO6/7 — das ist aber der I2C-Bus.
> Der HX711 ist **kein I2C-Gerät** und würde den Bus (Display + 5 Sensoren) stören.
> Deshalb in dieser Applikation auf **GPIO21 (DT) / GPIO22 (SCK)** verlegt. Bitte die Waage
> entsprechend verdrahten.

> 💡 **Zur Frage I2C-Pull-ups (2.2 kΩ auf SDA/SCL nach 3.3V):** Grundsätzlich sinnvoll,
> **aber** die meisten Breakout-Boards (BH1750, BMP280, SCD41, OLED) haben bereits Pull-ups
> onboard. Hängen viele Module am Bus, summieren sich diese parallel und ziehen den Bus zu
> stark. Empfehlung: **ein einziges** Paar 2.2–4.7 kΩ genügt — nicht pro Sensor. Bei kurzen
> Breadboard-Strecken oft ganz verzichtbar. Treten I2C-Aussetzer auf, ist ein Pull-up-Paar
> der erste Versuch.

### Steckplan

Der Fritzing-Steckplan liegt im Repository (`steckplan.png`). Erstellt mit
[Fritzing](https://fritzing.org). Fritzing-Parts für die Lehrveranstaltung:
[Interaktive-Medien / 15_Intro_Projektdoku](https://github.com/Interaktive-Medien/im_physical_computing/tree/main/15_Intro_Projektdoku).

*(Achtung: Im Steckplan liegt der HX711 noch auf GPIO6/7 — in der Software auf GPIO21/22
korrigiert, siehe Warnung oben.)*

### Umgang mit LittleFS (Website hochladen)

Die Website (HTML/CSS/JS) wird **getrennt vom Code** als Datei-Image hochgeladen:

1. Plugin **arduino-littlefs-upload** installieren (siehe Link oben). Video-Tutorial:
   <https://youtu.be/Qw5mQus-WwA>
2. Die Dateien liegen im Ordner **`mc/data/`** (`index.html`, `css/style.css`, `js/script.js`).
3. In der Arduino IDE: `mc.ino` öffnen → **CMD + SHIFT + P** → **„Upload LittleFS to
   Pico/ESP8266/ESP32"**.
4. Dabei wird der komplette `data/`-Ordner auf den ESP gespiegelt. Alte Dateien auf dem ESP
   werden gelöscht — nicht mehr benötigte Dateien müssen also nicht manuell entfernt werden.

### Kalibration (Gewicht & Lautstärke)

Zwei Sensoren brauchen einen **einmalig** zu ermittelnden Kalibrationswert, der persistent
in **Preferences** (Namespace `calib`) gespeichert und beim Start automatisch geladen wird.

**Gewicht (HX711)** — `calibration/gewicht_kalibrieren.ino`
1. Sketch flashen, seriellen Monitor (115200) öffnen.
2. Waage **leer** lassen → Tare (Nullpunkt).
3. Bekanntes Referenzgewicht auflegen (Standard 100 g, im Sketch `REF_WEIGHT_G` anpassbar).
4. `calFactor` wird berechnet und unter `calib / kf_gewicht` gespeichert.

**Lautstärke (INMP441)** — `calibration/lautstaerke_kalibrieren.ino`
1. Sketch flashen, seriellen Monitor öffnen — der **Roh-dB-Wert** wird laufend angezeigt.
2. Gleichzeitig den realen Pegel mit einem Smartphone-dB-Messgerät messen (z. B. App
   „Dezibel X").
3. Den gemessenen realen Wert (z. B. `58`) im Serial Monitor eintippen + Enter.
4. Der **Shift-Offset** (`realDB − db_raw`) wird berechnet und unter `calib / kf_lautst`
   gespeichert.

> Hinweis: Die NVS-Keys heißen aus technischen Gründen kurz `kf_gewicht` / `kf_lautst`
> (NVS-Schlüssel sind auf 15 Zeichen begrenzt). Sie entsprechen den konzeptionellen
> Variablen `kalibrationsfaktor_gewicht` bzw. `kalibrationsfaktor_lautstaerke`.

### Schritt-für-Schritt: Erstinbetriebnahme

1. Repository klonen, Libraries installieren (siehe Tabelle).
2. *(Optional, empfohlen)* Kalibrations-Sketches aus `calibration/` flashen → Faktoren setzen.
3. `mc/data/` per **„Upload LittleFS to ESP"** hochladen.
4. **`mc/mc.ino`** flashen.
5. **Erststart ohne Credentials:** ESP öffnet AP **`apibox`** (offen). Mit dem Handy/Laptop
   verbinden → Setup-Seite öffnet automatisch (Captive Portal) → Reiter **Heimnetzwerk**
   oder **Schulnetzwerk** wählen, Zugangsdaten eingeben, speichern.
6. ESP startet neu und verbindet sich. Das **Display** zeigt `Verbunden mit <SSID>` und die
   **IP-Adresse**. Die Status-LED leuchtet **grün**.
7. Im Browser testen: `http://<IP>/temperatur`, `http://<IP>/` (Dashboard), `http://<IP>/api`
   (Liste aller Endpunkte).
8. Netzwerk später wechseln: **Taster (GPIO20)** drücken → Captive Portal startet erneut.

---

## Technische Details

### Wie reden die Dateien miteinander?

* **`mc.ino`** ist die Zentrale: initialisiert I2C, Display, alle Sensoren und das WLAN,
  registriert die API-Routen und betreibt im `loop()` den Webserver. Jede Sensorlogik ist in
  eine eigene **`.h`-Datei** ausgelagert; `mc.ino` bindet sie per `#include` ein und ruft je
  Endpunkt eine `getXxx()`-Funktion auf (mit Kommentar, in welcher Datei sie steht).
* In der **Arduino IDE** erscheinen alle `.h`-Dateien im `mc/`-Ordner automatisch als **Tabs**
  und werden gemeinsam mit `mc.ino` kompiliert. Die Ordner `tests/`, `calibration/` und
  `examples/` liegen **daneben** (nicht in `mc/`) und werden daher **nicht** mitkompiliert.

### Weg der Daten

```
HTTP GET /temperatur
        │
        ▼
mc.ino  →  server.on("/temperatur")  →  getTemperatur()      [temperatur.h]
                                              │
                                              ▼
                                   readSCD41()  liest I2C-Sensor (gecacht, ≥5 s)
                                              │
                                              ▼
                                   Arduino_JSON baut {wert,einheit,datentyp,sensor}
                                              │
                                              ▼
                                   server.send(200, "application/json", …)
        ◀─────────────────────────────────────┘
   JSON-Antwort an den Client
```

### Besondere Entscheidungen

* **SCD41 wird geteilt:** Ein Sensor liefert Temperatur, Luftfeuchtigkeit **und** CO2.
  `readSCD41()` (in `temperatur.h`) liest alle drei Werte gebündelt und cached sie;
  `luftfeuchtigkeit.h` und `co2.h` greifen auf den Cache zu. Das verhindert mehrfache
  I2C-Zugriffe und Konflikte.
* **BMP280 wird geteilt:** Ein `Adafruit_BMP280`-Objekt in `luftdruck.h` wird von `hoehe.h`
  mitbenutzt (deshalb `luftdruck.h` vor `hoehe.h` einbinden).
* **On-demand:** Werte werden nur bei einer Anfrage gelesen — kein Dauer-Logging.
* **WLAN-Credentials & Kalibrationsfaktoren** liegen persistent in **Preferences** und
  überleben Neustarts/Stromausfall.

### Datenschnittstelle (zwischen WebApp und Physical Computing)

* **Format:** JSON, immer mit den Feldern `wert`, `einheit`, `datentyp`, `sensor`.
* **Transport:** HTTP GET, ein Endpunkt pro Messwert.
* **Client-seitig:** `examples/client_fetch.html` zeigt den Abruf mit `fetch()`.
* **Server-seitig:** `examples/server_proxy.php` zeigt den Abruf mit `curl_init` /
  `curl_setopt` / `curl_exec` / `curl_close` (nützlich als CORS-Proxy und zum Speichern in
  einer Datenbank).

### API-Übersicht

| Endpunkt | Bauteil | `datentyp` | `einheit` | Beispiel `wert` |
|---|---|---|---|---|
| `/temperatur` | SCD41 | float | °C | 22.7 |
| `/luftfeuchtigkeit` | SCD41 | float | % | 48.3 |
| `/co2` | SCD41 | int | ppm | 500 |
| `/bewegung` | SR602 | boolean | bewegung | true |
| `/lautstaerke` | INMP441 | float | dB | 58.4 |
| `/magnet` | GPS14-B | boolean | magnet_erkannt | false |
| `/helligkeit` | BH1750 | float | lx | 423.7 |
| `/alkohol` | MQ-3 | float | mg/L | 0.12 |
| `/lage` | ICM-20948 | object<float,float,float> | ° | {x,y,z} |
| `/gewicht` | HX711 | float | kg | 2.37 |
| `/rauch` | MQ-2 | int | analog_value | 350 |
| `/luftdruck` | BMP280 | float | hPa | 1013.25 |
| `/hoehe` | BMP280 | float | m | 450.2 |
| `/distanz` | VL53L0X | int | mm | 342 |
| `/api` | – | – | – | Liste aller Endpunkte |

---

## Organisation

### Projektstruktur

```
ESP_Sensors/
├── README.md                       ← diese Datei
├── steckplan.png                   ← Fritzing-Steckplan (hier ablegen)
│
├── mc/                             ← HAUPT-SKETCH (dieser Ordner wird kompiliert)
│   ├── mc.ino                     ← Einstieg: WLAN, Webserver, API-Routen, Loop
│   ├── display.h                  ← SSD1306 OLED
│   ├── captive_portal.h           ← AP "apibox", DNS, Setup-Formular, Credentials, Taster
│   ├── temperatur.h               ← SCD41 (gemeinsames readSCD41)
│   ├── luftfeuchtigkeit.h         ← SCD41
│   ├── co2.h                      ← SCD41
│   ├── bewegung.h                 ← SR602 PIR
│   ├── lautstaerke.h              ← INMP441 (+ Kalibrationsfaktor)
│   ├── magnet.h                   ← GPS14-B Reed
│   ├── helligkeit.h               ← BH1750
│   ├── alkohol.h                  ← MQ-3
│   ├── lage.h                     ← ICM-20948
│   ├── gewicht.h                  ← HX711 (+ Kalibrationsfaktor)
│   ├── rauch.h                    ← MQ-2
│   ├── luftdruck.h                ← BMP280
│   ├── hoehe.h                    ← BMP280
│   ├── distanz.h                  ← VL53L0X
│   └── data/                      ← per LittleFS hochladen
│       ├── index.html             ← Dashboard (fetch() aller Endpunkte)
│       ├── css/style.css
│       └── js/script.js
│
├── tests/                         ← getestete Original-Demo-Codes je Sensor/Technik
│   ├── temperatur.ino  luftfeuchtigkeit.ino  co2.ino  bewegung.ino
│   ├── lautstaerke.ino  magnet.ino  helligkeit.ino  alkohol.ino  lage.ino
│   ├── gewicht.ino  rauch.ino  luftdruck.ino  hoehe.ino  distanz.ino
│   ├── oled_display.ino  button.ino  preferences.ino
│   └── captive_portal.ino  littlefs_webserver.ino
│
├── calibration/                   ← einmalige Kalibrationsprogramme
│   ├── gewicht_kalibrieren.ino
│   └── lautstaerke_kalibrieren.ino
│
└── examples/                      ← API-Abruf von aussen
    ├── client_fetch.html          ← client-seitig (JavaScript fetch)
    └── server_proxy.php           ← server-seitig (PHP cURL)
```

Jede Code-Datei hat im **Kopfbereich** eine kurze Zusammenfassung (Zweck, benötigte Library,
Verkabelung). `mc.ino` enthält zusätzlich den vollständigen Pinplan.

---

## Known Bugs / Hinweise

* **MQ-3 / MQ-2 & ADC-Spannung:** Beide Sensoren laufen mit **5V** am Heizer; ihr Analogausgang
  (AO) kann über 3.3V steigen und den ESP32-Eingang sättigen/belasten. Für saubere Werte einen
  **Spannungsteiler** (z. B. 2×10 kΩ) vor den ADC-Pin setzen. Beide Sensoren liegen auf
  **ADC1** (ADC2 ist bei aktivem WLAN nicht nutzbar).
* **MQ-Aufwärmzeit:** MQ-3/MQ-2 brauchen 1–5 Minuten Vorheizen für stabile Werte; direkt nach
  dem Start sind die Werte zu hoch.
* **Alkohol in mg/L** ist nur eine **grobe lineare Näherung** aus dem Rohwert — ohne echte
  Gas-Kalibration nicht messtechnisch belastbar.
* **SCD41-Intervall:** Der Sensor misst intern ca. alle 5 s. Sehr schnelle aufeinanderfolgende
  Abfragen liefern denselben (gecachten) Wert.
* **ICM-20948 `autoOffsets()`** läuft beim Start — das Board währenddessen **flach und ruhig**
  halten, sonst sind die Winkel verschoben.
* **HX711-Verdrahtung:** muss auf GPIO21/22 liegen (nicht 6/7), sonst kollidiert die Waage mit
  dem I2C-Bus (siehe Warnung oben).
* **CORS:** Der ESP setzt keinen `Access-Control-Allow-Origin`-Header. Für Cross-Origin-Zugriffe
  aus dem Browser entweder die Seite über die API Box selbst ausliefern oder den
  PHP-Proxy (`server_proxy.php`) verwenden.
* **Captive Portal offen:** Der Access Point `apibox` ist absichtlich ohne Passwort — nur für
  die Einrichtung gedacht.

### Was könnte noch verbessert werden?

* Echte Kalibrationskurven für die Gassensoren (MQ-3/MQ-2).
* Optionaler `Access-Control-Allow-Origin: *`-Header für direkten Browser-Zugriff.
* Mittelwertbildung/Glättung pro Endpunkt konfigurierbar machen.
* Absicherung der API (Token) für den Produktivbetrieb.
