/**********************************************************************************************
 *  gewicht.h
 *  Waegesensor mit HX711 Treibermodul
 *  Library: HX711 Arduino Library by Bogdan Necula
 *
 *  WICHTIG — Anschluss (NICHT auf dem I2C-Bus!):
 *  HX711: DT (Data)   <->  ESP32-C6: GPIO21
 *  HX711: SCK (Clock) <->  ESP32-C6: GPIO22
 *  HX711: VCC         <->  ESP32-C6: 3.3V
 *  HX711: GND         <->  ESP32-C6: GND
 *
 *  HX711 ist KEIN I2C-Geraet. Im urspruenglichen Steckplan lag es auf GPIO6/7 —
 *  das ist aber der I2C-Bus (Display + SCD41 + BH1750 + ICM-20948 + BMP280 + VL53L0X).
 *  Daher auf GPIO21/22 verschoben, damit alle Sensoren gleichzeitig funktionieren.
 *
 *  Kalibration: Der Kalibrationsfaktor wird persistent in Preferences gespeichert
 *  (Namespace "calib", Key "kf_gewicht" — NVS-Keys sind auf 15 Zeichen begrenzt,
 *  daher Kurzform der Variable "kalibrationsfaktor_gewicht").
 *  Ermittlung einmalig mit calibration/gewicht_kalibrieren.ino.
 **********************************************************************************************/

#ifndef GEWICHT_H
#define GEWICHT_H

#include "HX711.h"

#define HX711_DT  21
#define HX711_SCK 22

HX711 scale;
float kalibrationsfaktor_gewicht = 1.0;   // wird aus Preferences geladen

void setupGewicht() {
  scale.begin(HX711_DT, HX711_SCK);
  scale.set_gain(128);

  // Kalibrationsfaktor aus Preferences laden
  preferences.begin("calib", true);                                   // global aus mc.ino
  kalibrationsfaktor_gewicht = preferences.getFloat("kf_gewicht", 1.0);
  preferences.end();

  scale.set_scale(kalibrationsfaktor_gewicht);
  scale.tare(15);   // Nullpunkt setzen (nichts auflegen beim Start)

  Serial.print("HX711 initialisiert. calFactor = ");
  Serial.println(kalibrationsfaktor_gewicht);
}

String getGewicht() {
  // get_units() liefert Gramm (calFactor wurde mit g-Referenz ermittelt) -> /1000 = kg
  float kg = 0.0;
  if (scale.is_ready()) {
    kg = scale.get_units(10) / 1000.0;
  }

  return createJsonResponse(String(kg, 2), "kg", "float", "HX711");
}

#endif
