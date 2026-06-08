/**********************************************************************************************
*  distanz.ino  (Test/Demo)
*  VL53L0X Time-of-Flight Distanzsensor (TOF200C) — Distanz in mm.
*  Referenz fuer mc/distanz.h.
*  Library: Adafruit VL53L0X Library.
*  Sensor: VIN<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
*  Messbereich ca. 30-2000 mm. Status 4 = ausser Reichweite.
***********************************************************************************************/

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  if (!lox.begin()) {
    Serial.println("VL53L0X nicht gefunden!");
    while (1);
  }
  Serial.println("VL53L0X bereit.");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    Serial.print("Distanz: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println("ausser Reichweite");
  }
  delay(200);
}
