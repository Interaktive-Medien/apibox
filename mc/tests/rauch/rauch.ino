/**********************************************************************************************
* rauch.ino  (Test/Demo)
* MQ-2 Gas- und Rauchsensor — analoger Rohwert.
* Original-Demo MQ-2.ino — Referenz fuer mc/rauch.h.
* Keine externe Library erforderlich.
* Sensor: VCC<->5V (Heizelement!)  GND<->GND  AO<->GPIO0  DO<->nicht verbunden
*
* Typische Werte (12-Bit ADC 0-4095):
*  < 500 = saubere Raumluft, 500-1200 = leichte Verunreinigung, > 1200 = moegl. Alarm
***********************************************************************************************/

#define MQ2_ANALOG_PIN 0

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  Serial.println("MQ-2 Sensor gestartet (Aufwaermphase beachten)");
}

void loop() {
  int gasValue = analogRead(MQ2_ANALOG_PIN);
  Serial.print("Gas/Rauch Analogwert: ");
  Serial.println(gasValue);
  delay(1000);
}
