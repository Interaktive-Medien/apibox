/******************************************************************************
 *  luftdruck.h
 *  BMP280 Luftdrucksensor via I2C
 *  Library: Adafruit BMP280 Library (+ Adafruit Unified Sensor)
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *****************************************************************************/

#ifndef LUFTDRUCK_H
#define LUFTDRUCK_H

#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // gemeinsames Objekt fuer Luftdruck + Hoehe

// aufgerufen in mc.ino
void setupLuftdruck()
{
  if (!bmp.begin(0x76))
  {
    Serial.println(F("Kein BMP280 gefunden!"));
    while (1)
      delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

// aufgerufen in mc.ino
float getLuftdruck()
{
  float luftdruck = bmp.readPressure() / 100.0F;
  return luftdruck;
}

#endif