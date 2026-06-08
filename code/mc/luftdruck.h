/**********************************************************************************************
 *  luftdruck.h
 *  BMP280 Luftdrucksensor via I2C
 *  Library: Adafruit BMP280 Library (+ Adafruit Unified Sensor)
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *
 *  Hinweis: Das hier erzeugte Objekt "bmp" wird auch von hoehe.h mitbenutzt.
 *  luftdruck.h MUSS daher vor hoehe.h eingebunden werden (Reihenfolge in mc.ino).
 **********************************************************************************************/

#ifndef LUFTDRUCK_H
#define LUFTDRUCK_H

#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;            // gemeinsames Objekt fuer Luftdruck + Hoehe
bool bmpInitialized = false;

// Referenz-Luftdruck auf Meereshoehe (fuer Hoehenberechnung in hoehe.h)
const float SEALEVEL_HPA = 1013.25;

void setupLuftdruck() {
  // BMP280 I2C-Adresse meist 0x76, manche Boards 0x77
  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
    bmpInitialized = true;
    Serial.println("BMP280 initialisiert.");
  } else {
    bmpInitialized = false;
    Serial.println("BMP280 nicht gefunden!");
  }
}

String getLuftdruck() {
  float hPa = bmpInitialized ? (bmp.readPressure() / 100.0F) : -1.0;
  return createJsonResponse(String(hPa, 2), "hPa", "float", "BMP280");
}

#endif
