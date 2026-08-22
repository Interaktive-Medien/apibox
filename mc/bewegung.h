/**********************************************************************************************
 *  bewegung.h
 *  SR602 PIR Bewegungssensor
 *  Keine Library erforderlich.
 *
 *  Anschluss:
 *  Sensor: VCC   <->  ESP32-C6: 3.3V
 *  Sensor: GND   <->  ESP32-C6: GND
 *  Sensor: Data  <->  ESP32-C6: GPIO4
 **********************************************************************************************/

#ifndef BEWEGUNG_H
#define BEWEGUNG_H

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

const int pirPin = 4;
int pir_state = 0;

// aufgerufen in mc.ino
void setupBewegung()
{
  pinMode(pirPin, INPUT);
  Serial.println("PIR SR602 initialisiert.");
}

// aufgerufen in mc.ino
int getBewegung()
{
  pir_state = digitalRead(pirPin);
  return pir_state;
}

#endif
