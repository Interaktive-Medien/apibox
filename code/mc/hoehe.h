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

String getHoehe() {
  float meter = bmpInitialized ? bmp.readAltitude(SEALEVEL_HPA) : -1.0;   // bmp, SEALEVEL_HPA aus luftdruck.h
  return createJsonResponse(String(meter, 2), "m", "float", "BMP280");
}

#endif
