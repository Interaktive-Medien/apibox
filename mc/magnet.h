/******************************************************************************
 *  magnet.h
 *  GPS14-B Magnetsensor (Reed-Kontakt) — Tuer offen/zu Erkennung
 *  Keine Library erforderlich.
 *
 *  Anschluss:
 *  Sensor: Pin1  <->  ESP32-C6: GPIO11
 *  Sensor: Pin2  <->  ESP32-C6: GND
 *  Sensor: Pin1<->GPIO11  Pin2<->GND   (INPUT_PULLDOWN: HIGH = Magnet erkannt)
 *****************************************************************************/

#ifndef MAGNET_H
#define MAGNET_H

const int magnetPin = 11;
int magnetstate = 0;

// aufgerufen in mc.ino
void setupMagnet()
{
  pinMode(magnetPin, INPUT_PULLDOWN);
  Serial.println("Magnetsensor GPS14-B initialisiert.");
}

// aufgerufen in mc.ino
int getMagnet()
{
  magnetstate = digitalRead(magnetPin);
  return magnetstate;
}

#endif
