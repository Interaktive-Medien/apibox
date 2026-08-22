/******************************************************************************
 *  gewicht.h
 *  Waegesensor mit HX711 Treibermodul
 *  Library: Adafruit HX711 Library by Adafruit z.B. V1.0.2
 *
 *  WICHTIG — Anschluss (NICHT auf dem I2C-Bus!):
 *  HX711: DT (Data)   <->  ESP32-C6: GPIO21
 *  HX711: SCK (Clock) <->  ESP32-C6: GPIO22
 *  HX711: VCC         <->  ESP32-C6: 3.3V
 *  HX711: GND         <->  ESP32-C6: GND
 *
 *  HX711 ist KEIN I2C-Gerät.

 *  Kalibration: Der Kalibrationsfaktor wurde persistent in Preferences gespeichert und wird hier aufgerufen
 *  Ermittlung einmalig mit tests/gewicht/01_calib.ino.
 *****************************************************************************/

#ifndef GEWICHT_H
#define GEWICHT_H

#include "Adafruit_HX711.h"

const int HX_DT = 21;
const int HX_SCK = 22;

Adafruit_HX711 hx711(HX_DT, HX_SCK);

int32_t tareValue = 0;
float waage_calfactor = 1.0f;

// aufgerufen in mc.ino
void setupGewicht()
{
  // 1. Gespeicherten Faktor abrufen
  preferences.begin("waage", true); // true = reiner Lesezugriff (schont den Flash)
  // Lese "calfactor". Falls er nicht existiert, nimm als Fallback 1.0f
  waage_calfactor = preferences.getFloat("calfactor", 1.0f);
  preferences.end();

  Serial.print("Geladener Kalibrierungsfaktor: ");
  Serial.println(waage_calfactor);

  if (waage_calfactor == 1.0f)
  {
    Serial.println("WARNUNG: Faktor ist 1.0. Wurde das Kalibrierungsprogramm ausgeführt?");
  }

  hx711.begin();

  // 2. Waage nullen (Automatisches Tare beim Einschalten)
  Serial.println("Waage nullt sich... (bitte nichts auflegen)");
  for (int i = 0; i < 3; i++)
    hx711.readChannelBlocking(CHAN_A_GAIN_128); // Stabilisieren

  int64_t sum = 0;
  for (int i = 0; i < 15; i++)
  {
    sum += hx711.readChannelBlocking(CHAN_A_GAIN_128);
  }
  tareValue = sum / 15;

  Serial.println("Waage ist bereit!");
}

// aufgerufen in mc.ino
float getGewicht()
{
  // Aktuellen Rohwert lesen
  int32_t rawValue = hx711.readChannelBlocking(CHAN_A_GAIN_128);

  // Umrechnung: (Aktueller Wert - Nullpunkt) / Faktor
  float grams = (rawValue - tareValue) / waage_calfactor;
  return grams;
}
#endif
