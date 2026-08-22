/******************************************************************************
 *  helligkeit.h
 *  BH1750 Lichtsensor (Beleuchtungsstaerke in Lux) via I2C
 *  Library: BH1750 von Christopher Laws
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *****************************************************************************/

#ifndef HELLIGKEIT_H
#define HELLIGKEIT_H

#include <BH1750.h>

BH1750 lightMeter;
bool bh1750Initialized = false;

// aufgerufen in mc.ino
void setupHelligkeit()
{
  // Wire.begin() erfolgt zentral in mc.ino
  if (lightMeter.begin())
  {
    Serial.println("BH1750 initialisiert.");
    bh1750Initialized = true;
  }
  else
  {
    Serial.println("BH1750 nicht gefunden!");
    bh1750Initialized = false;
  }
}

// aufgerufen in mc.ino
float getHelligkeit()
{
  float lux = bh1750Initialized ? lightMeter.readLightLevel() : -1.0;
  return lux;
}

#endif
