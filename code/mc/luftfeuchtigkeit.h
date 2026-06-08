/**********************************************************************************************
 *  luftfeuchtigkeit.h
 *  SCD41 Luftfeuchtigkeit via I2C
 *  Library: Sensirion I2C SCD4x (bereits in temperatur.h initialisiert)
 *
 *  Anschluss: I2C (GPIO6: SDA, GPIO7: SCL)
 *  Nutzt die gemeinsamen SCD41-Variablen aus temperatur.h
 **********************************************************************************************/

#ifndef LUFTFEUCHTIGKEIT_H
#define LUFTFEUCHTIGKEIT_H

String getLuftfeuchtigkeit() {
  readSCD41();  // Funktion in temperatur.h
  return createJsonResponse(String(scd41_humidity, 2), "%", "float", "SCD41");
}

#endif
