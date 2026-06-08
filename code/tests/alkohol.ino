/**********************************************************************************************
*  alkohol.ino  (Test/Demo)
*  MQ-3 Alkohol-Gassensor — analoger Messwert (A0).
*  Original-Demo MQ-3.ino — Referenz fuer mc/alkohol.h.
*  Keine Library noetig. Werte ohne Kalibrierung nicht direkt als mg/L interpretierbar.
*  Sensor: VCC<->5V  GND<->GND  A0<->GPIO5  D0<->nicht angeschlossen
*  Hinweis: Heizung benoetigt 1-5 Min. Aufwaermzeit fuer stabile Messungen.
***********************************************************************************************/

const int MQ3_PIN = 5;  // GPIO5

void setup() {
  Serial.begin(115200);
}

void loop() {
  int adc = analogRead(MQ3_PIN);
  Serial.print("MQ-3 ADC: ");
  Serial.println(adc);
  delay(500);
}
