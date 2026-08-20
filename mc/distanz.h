/**********************************************************************************************
 *  distanz.h
 *  VL53L0X Time-of-Flight Distanzsensor (TOF200C) via I2C
 *  Library: Adafruit VL53L0X Library
 *
 *  Anschluss:
 *  Sensor: VIN  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *
 *  Messbereich VL53L0X: ca. 30 - 2000 mm. Status 4 = ausser Reichweite.
 **********************************************************************************************/

#ifndef DISTANZ_H
#define DISTANZ_H

#include <Adafruit_VL53L0X.h>

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool loxInitialized = false;

// aufgerufen in mc.ino
void setupDistanz()
{
  if (lox.begin())
  {
    loxInitialized = true;
    Serial.println("VL53L0X initialisiert.");
  }
  else
  {
    loxInitialized = false;
    Serial.println("VL53L0X nicht gefunden!");
  }
}

// aufgerufen in mc.ino
String getDistanz()
{
  int distance = -1;
  if (loxInitialized)
  {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus != 4)
    { // 4 = out of range
      distance = measure.RangeMilliMeter;
    }
  }

  return createJsonResponse(String(distance), "mm", "int", "VL53L0X"); // in mc.ino
}

#endif
