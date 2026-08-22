/**********************************************************************************************
 *  distanz.ino  (Test/Demo)
 *  VL53L0X Time-of-Flight Distanzsensor (TOF200C) — Distanz in mm.
 *  Referenz fuer mc/distanz.h.
 *  Library: VL53L0X by Pololu V1.3.1
 *  Sensor: VIN<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
 *  Messbereich ca. 30-2000 mm. Status 4 = ausser Reichweite.
 ***********************************************************************************************/

#ifndef DISTANZ_H
#define DISTANZ_H

#include <VL53L0X.h>
// Sensor-Instanz erstellen
VL53L0X sensor;

// aufgerufen in mc.ino
void setupDistanz()
{
  // Sensor initialisieren
  if (!sensor.init())
  {
    Serial.println(F("VL53L0X nicht gefunden!"));
    while (1)
      ;
  }
}

// aufgerufen in mc.ino
int getDistanz()
{
  // Entfernung messen
  uint16_t distance = sensor.readRangeSingleMillimeters();

  if (sensor.timeoutOccurred())
  {
    Serial.println(F("Zeitüberschreitung bei Messung!"));
  }
  return distance;
}

#endif
