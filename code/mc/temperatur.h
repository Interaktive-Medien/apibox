/**********************************************************************************************
 *  temperatur.h
 *  SCD41 Temperatur-Sensor via I2C
 *  Library: Sensirion I2C SCD4x + Sensirion Core
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *
 *  Hinweis: SCD41 liefert auch Luftfeuchtigkeit und CO2 (siehe luftfeuchtigkeit.h, co2.h).
 *  Alle drei Werte werden gemeinsam ausgelesen, um doppelte I2C-Zugriffe zu vermeiden.
 **********************************************************************************************/

#ifndef TEMPERATUR_H
#define TEMPERATUR_H

#include <SensirionI2cScd4x.h>

SensirionI2cScd4x scd4x;
bool scd4xInitialized = false;

// Gemeinsame Variablen fuer SCD41 (Temperatur, Luftfeuchtigkeit, CO2)
float scd41_temperature = 0.0;
float scd41_humidity = 0.0;
uint16_t scd41_co2 = 0;
unsigned long scd41_lastRead = 0;
const unsigned long SCD41_READ_INTERVAL = 5000;  // Min. 5s zwischen Messungen

void setupTemperatur() {
  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  uint16_t error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.println("SCD41 Start fehlgeschlagen!");
    scd4xInitialized = false;
  } else {
    Serial.println("SCD41 gestartet.");
    scd4xInitialized = true;
  }
}

// Gemeinsames Auslesen aller SCD41-Werte
void readSCD41() {
  if (!scd4xInitialized) return;
  if (millis() - scd41_lastRead < SCD41_READ_INTERVAL) return;

  bool dataReady = false;
  scd4x.getDataReadyStatus(dataReady);
  if (!dataReady) return;

  scd4x.readMeasurement(scd41_co2, scd41_temperature, scd41_humidity);
  scd41_lastRead = millis();
}

String getTemperatur() {
  readSCD41();
  return createJsonResponse(String(scd41_temperature, 2), "°C", "float", "SCD41");
}

#endif
