/**********************************************************************************************
 *  hoehe.h
 *  BMP280 Hoehenberechnung (aus Luftdruck) via I2C
 *  Library: Adafruit BMP280 Library (Objekt "bmp" stammt aus luftdruck.h)
 *
 *  Anschluss: I2C (GPIO6: SDA, GPIO7: SCL)
 *  Nutzt das gemeinsame bmp-Objekt + SEALEVEL_HPA aus luftdruck.h.
 *  -> luftdruck.h muss vor hoehe.h eingebunden werden.
 **********************************************************************************************/

#ifndef HOEHE_H
#define HOEHE_H

#include <Adafruit_BMP280.h>

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

// Externer Zugriff auf Objekte in luftdruck.h
extern Adafruit_BMP280 bmp;
extern bool bmpInitialized;
extern const float SEALEVEL_HPA;

// aufgerufen in mc.ino
String getHoehe()
{
  float meter = bmpInitialized ? bmp.readAltitude(SEALEVEL_HPA) : -1.0; // in luftdruck.h
  return createJsonResponse(String(meter, 2), "m", "float", "BMP280");  // in mc.ino
}

#endif
