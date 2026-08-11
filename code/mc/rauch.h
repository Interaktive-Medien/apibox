/**********************************************************************************************
 *  rauch.h
 *  MQ-2 Gas- und Rauchsensor (analog)
 *  Keine Library erforderlich (analogRead).
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 5V (Heizelement)
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: AO   <->  ESP32-C6: GPIO0 (Analog, ADC1)
 *  Sensor: DO   <->  nicht verbunden
 *
 *  Typische Werte (12-Bit ADC, 0-4095):
 *    < 500     = saubere Raumluft
 *    500-1200  = leichte Verunreinigung / Aufwaermphase
 *    > 1200    = moeglicher Gas-/Rauchalarm
 **********************************************************************************************/

#ifndef RAUCH_H
#define RAUCH_H

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

// aufgerufen in mc.ino
void setupRauch()
{
  analogReadResolution(12); // 0-4095
  Serial.println("MQ-2 Rauchsensor initialisiert.");
}

// aufgerufen in mc.ino
String getRauch()
{
  int gasValue = analogRead(PIN_MQ2);
  return createJsonResponse(String(gasValue), "analog_value", "int", "MQ-2"); // in mc.ino
}

#endif
