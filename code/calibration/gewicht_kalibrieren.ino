/******************************************************************************************************
 *  gewicht_kalibrieren.ino
 *  Einmalige Kalibration des HX711-Waegesensors.
 *  Ermittelt den Kalibrationsfaktor (calFactor) und speichert ihn PERSISTENT in Preferences,
 *  damit mc.ino / gewicht.h ihn beim Start laden kann.
 *
 *  Library: "HX711 Arduino Library" by Bogdan Necula
 *  Anschluss (wie in der finalen Applikation, NICHT auf dem I2C-Bus):
 *    HX711: DT (Data)  <->  ESP32-C6: GPIO21
 *    HX711: SCK (Clock)<->  ESP32-C6: GPIO22
 *    HX711: VCC        <->  ESP32-C6: 3.3V
 *    HX711: GND        <->  ESP32-C6: GND
 *
 *  Preferences:  Namespace "calib", Key "kf_gewicht"
 *  (Kurzform der konzeptionellen Variable "kalibrationsfaktor_gewicht";
 *   NVS-Keys sind auf 15 Zeichen begrenzt.)
 *
 *  ABLAUF (Serieller Monitor, 115200 Baud):
 *  1. Beim Start NICHTS auf die Waage legen -> Tare (Nullpunkt).
 *  2. Nach Aufforderung ein bekanntes Referenzgewicht auflegen (REF_WEIGHT_G unten anpassen).
 *  3. calFactor wird berechnet und gespeichert. Danach laeuft die finale mc.ino mit diesem Wert.
 ******************************************************************************************************/

#include "HX711.h"
#include <Preferences.h>

#define HX711_DT  21
#define HX711_SCK 22

// Bekanntes Referenzgewicht in GRAMM (z.B. 100 g Schokolade). Bitte anpassen!
const float REF_WEIGHT_G = 100.0;

HX711 scale;
Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(1000);

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_gain(128);
  delay(200);

  Serial.println("=== HX711 Kalibration ===");
  Serial.println("Schritt 1: Waage LEER lassen. Tare in 3 Sekunden...");
  delay(3000);
  scale.tare(20);
  Serial.println("Nullpunkt (Tare) gesetzt.");

  Serial.print("Schritt 2: Lege jetzt das Referenzgewicht auf (");
  Serial.print(REF_WEIGHT_G);
  Serial.println(" g). 8 Sekunden Zeit...");
  delay(8000);

  long raw = scale.get_value(20);            // Rohwert ohne Skalierung (gemittelt)
  float calFactor = raw / REF_WEIGHT_G;      // calFactor in Einheit "pro Gramm"

  // Persistent speichern
  preferences.begin("calib", false);
  preferences.putFloat("kf_gewicht", calFactor);
  preferences.end();

  Serial.println("=== Kalibration abgeschlossen ===");
  Serial.print("calFactor (kf_gewicht) = ");
  Serial.println(calFactor);
  Serial.println("Wert wurde in Preferences gespeichert.");
  Serial.println("Jetzt mc.ino flashen — der Faktor wird automatisch geladen.");

  // Kontrolle: skaliertes Gewicht anzeigen
  scale.set_scale(calFactor);
}

void loop() {
  // Zur Kontrolle: aktuelles Gewicht in Gramm anzeigen
  if (scale.is_ready()) {
    float grams = scale.get_units(10);
    Serial.print("Kontrolle: ");
    Serial.print(grams);
    Serial.println(" g");
  }
  delay(1000);
}
