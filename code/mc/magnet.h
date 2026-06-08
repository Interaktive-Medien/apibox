/**********************************************************************************************
 *  magnet.h
 *  GPS14-B Magnetsensor (Reed-Kontakt) — Tuer offen/zu Erkennung
 *  Keine Library erforderlich.
 *
 *  Anschluss:
 *  Sensor: Pin1  <->  ESP32-C6: GPIO11
 *  Sensor: Pin2  <->  ESP32-C6: GND
 *  (INPUT_PULLUP: HIGH = kein Magnet, LOW = Magnet erkannt)
 **********************************************************************************************/

#ifndef MAGNET_H
#define MAGNET_H

void setupMagnet() {
  pinMode(PIN_MAGNET, INPUT_PULLUP);
  Serial.println("Magnetsensor GPS14-B initialisiert.");
}

String getMagnet() {
  int state = digitalRead(PIN_MAGNET);
  // LOW = Magnet erkannt (Reed geschlossen), HIGH = kein Magnet
  bool magnetDetected = (state == LOW);
  return createJsonResponse(magnetDetected ? "true" : "false", "magnet_erkannt", "boolean", "GPS14-B");
}

#endif
