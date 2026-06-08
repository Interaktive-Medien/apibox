/**********************************************************************************************
 *  co2.h
 *  SCD41 CO2-Konzentration via I2C
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

#ifndef CO2_H
#define CO2_H

String getCO2() {
  readSCD41();  // Funktion in temperatur.h
  return createJsonResponse(String((int)scd41_co2), "ppm", "int", "SCD41");
}

#endif
