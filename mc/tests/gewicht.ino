/******************************************************************************
 *  gewicht.ino  (Test/Demo)
 *  Waegesensor mit HX711 Treibermodul.
 *  Original-Demo 17_Waage.ino — Referenz fuer mc/gewicht.h.
 *  Library: "HX711 Arduino Library" by Bogdan Necula
 *
 *  WICHTIG: In der finalen Applikation liegt HX711 auf GPIO21/22 (NICHT 6/7,
 *  das ist der I2C-Bus). Dieses Demo nutzt noch die Original-Pins 6/7 und
 *  sollte daher OHNE angeschlossene I2C-Sensoren getestet werden.
 *  HX711: DT<->GPIO6  SCK<->GPIO7  Vcc<->3.3V  GND<->GND
 *
 *  Kalibration: Bei Start nichts auflegen (Tare). Dann Referenzgewicht (z.B. 100g)
 *  auflegen -> calFactor wird berechnet.
 *******************************************************************************/

#include <Wire.h>
#include "HX711.h"

const int HX_DT = 6;   // DOUT
const int HX_SCK = 7;  // SCK
HX711 scale;

float calFactor = 1.0f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  scale.begin(HX_DT, HX_SCK);
  scale.set_gain(128);
  delay(200);

  scale.tare(15);
  Serial.println("Tare gesetzt");
  Serial.println("Lege bekanntes Gewicht auf");
  delay(5000);
}

void loop() {
  static bool calibrated = false;
  const float refWeight = 100.0; // 100 g Referenz

  if (!calibrated) {
    long raw = scale.get_value(15);
    calFactor = raw / refWeight;
    scale.set_scale(calFactor);
    calibrated = true;
    Serial.println("Kalibrierung OK");
    Serial.print("calFactor: ");
    Serial.println(calFactor);
    delay(2000);
  }

  float grams = scale.get_units(10);
  Serial.print(grams);
  Serial.println(" g");
  delay(500);
}
