/**********************************************************************************************
 *  co2_temperatur_luftfeuchtigkeit.h
 *  SCD41 CO2-Konzentration, temperatur, Luftfeuchtigkeit via I2C
 *  Library: Sensirion I2C SCD4x (bereits in temperatur.h initialisiert)
 *
 *  Anschluss: I2C (GPIO6: SDA, GPIO7: SCL)
 *  Nutzt die gemeinsamen SCD41-Variablen aus temperatur.h
 *
 *  Typische Werte:
 *    400 ppm  = Frischluft
 *    800 ppm  = gut beluefteter Raum
 *   1000 ppm  = durchschnittliches Buero
 *   2000 ppm  = schlechte Luft
 **********************************************************************************************/

#ifndef CO2_TEMPERATUR_LUFTFEUCHTIGKEIT_H
#define CO2_TEMPERATUR_LUFTFEUCHTIGKEIT_H

#include <SensirionI2cScd4x.h>

uint16_t co2;
float temperature;
float humidity;

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

SensirionI2cScd4x scd4x;
bool scd4xInitialized = false;

// Gemeinsame Variablen fuer SCD41 (Temperatur, Luftfeuchtigkeit, CO2)
float scd41_temperature = 0.0;
float scd41_humidity = 0.0;
uint16_t scd41_co2 = 0;
unsigned long scd41_lastRead = 0;
const unsigned long SCD41_READ_INTERVAL = 5000; // Min. 5s zwischen Messungen

// aufgerufen in mc.ino
void setupCo2_Temperatur_Luftfeuchtigkeit()
{
  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  // Teste I2C-Verbindung zum SCD41
  Wire.beginTransmission(0x62);
  int wireErr = Wire.endTransmission();
  Serial.print("Wire.endTransmission: ");
  Serial.println(wireErr);
  // Ende Test

  uint16_t error = scd4x.startPeriodicMeasurement();
  if (error)
  {
    Serial.println("SCD41 Start fehlgeschlagen!");
    scd4xInitialized = false;
  }
  else
  {
    Serial.println("SCD41 gestartet.");
    scd4xInitialized = true;
  }
}

// Gemeinsames Auslesen aller SCD41-Werte
void readSCD41()
{
  if (!scd4xInitialized)
  {
    // Serial.println("SCD41 nicht initialisiert!");
    return;
  }
  if (millis() - scd41_lastRead < SCD41_READ_INTERVAL)
    return;

  bool dataReady = false;
  scd4x.getDataReadyStatus(dataReady);
  if (!dataReady)
    return;

  scd4x.readMeasurement(scd41_co2, scd41_temperature, scd41_humidity); // Werte in Variablen oben speichern
  // Serial.println("scd41_temperature: " + String(scd41_temperature, 2));
  scd41_lastRead = millis();
}

// aufgerufen in mc.ino
void getCo2_Temperatur_Luftfeuchtigkeit()
{
  bool dataReady;
  scd4x.getDataReadyStatus(dataReady);
  if (dataReady)
  {
    scd4x.readMeasurement(co2, temperature, humidity);
  }
}

#endif
