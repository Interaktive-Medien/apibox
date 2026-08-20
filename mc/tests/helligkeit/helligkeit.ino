/**********************************************************************************************
*  helligkeit.ino  (Test/Demo)
*  BH1750 Lichtsensor — Beleuchtungsstaerke in Lux (lx).
*  Original-Demo BH1750.ino — Referenz fuer mc/helligkeit.h.
*  Library: BH1750 von Christopher Laws.
*  Sensor: VCC<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
***********************************************************************************************/

#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7); // SDA, SCL
  if (lightMeter.begin()) {
    Serial.println("BH1750 gefunden");
  } else {
    Serial.println("BH1750 nicht gefunden");
    while (true);
  }
}

void loop() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Lichtstaerke: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);
}
