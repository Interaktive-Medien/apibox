/**********************************************************************************************
 *  luftfeuchtigkeit.h
 *  SCD41 Luftfeuchtigkeit via I2C
 *  Library: Sensirion I2C SCD4x (bereits in temperatur.h initialisiert)
 *  Werte als JSON-String abrufen: http://[IP_ADRESSE]/luftfeuchtigkeit
 *
 *  Anschluss: I2C (GPIO6: SDA, GPIO7: SCL)
 *  Nutzt die gemeinsamen SCD41-Variablen aus temperatur.h
 **********************************************************************************************/

#ifndef LUFTFEUCHTIGKEIT_H
#define LUFTFEUCHTIGKEIT_H

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);
void readSCD41();            // in temperatur.h
extern float scd41_humidity; // in temperatur.h

// aufgerufen in mc.ino
String getLuftfeuchtigkeit()
{
  readSCD41(); // in temperatur.h
  return createJsonResponse(String(scd41_humidity, 2), "%", "float", "SCD41");
}

#endif
