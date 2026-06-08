/**********************************************************************************************
 *  alkohol.h
 *  MQ-3 Alkoholsensor (analog)
 *  Keine Library erforderlich (analogRead).
 *
 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 5V (Heizelement)
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: AO   <->  ESP32-C6: GPIO5 (Analog, ADC1)
 *
 *  Hinweis: Der Rohwert (0-4095 bei 12-Bit ADC) wird in eine grobe mg/L-Schaetzung
 *  umgerechnet. Fuer exakte Werte muesste der MQ-3 gegen bekannte Konzentrationen
 *  kalibriert werden — hier vereinfachte lineare Naeherung.
 **********************************************************************************************/

#ifndef ALKOHOL_H
#define ALKOHOL_H

void setupAlkohol() {
  analogReadResolution(12);   // 0-4095
  Serial.println("MQ-3 Alkoholsensor initialisiert.");
}

String getAlkohol() {
  int raw = analogRead(PIN_MQ3);

  // Vereinfachte Umrechnung Rohwert -> mg/L (grobe Naeherung).
  // 0 ... 4095  ->  0.00 ... ca. 2.00 mg/L
  float mgL = (raw / 4095.0) * 2.0;

  return createJsonResponse(String(mgL, 2), "mg/L", "float", "MQ-3");
}

#endif
