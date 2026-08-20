/**********************************************************************************************
*  luftfeuchtigkeit.ino  (Test/Demo)
*  SCD41 — Luftfeuchtigkeit (+ CO2, Temperatur) ueber I2C.
*  Demo-Code als Referenz fuer mc/luftfeuchtigkeit.h.
*  Library: Sensirion I2C SCD4x (+ Sensirion Core)
*  Sensor: VCC<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
***********************************************************************************************/

#include <Wire.h>
#include <SensirionI2cScd4x.h>

SensirionI2cScd4x sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  sensor.startPeriodicMeasurement();
  Serial.println("SCD4x gestartet");
}

void loop() {
  bool dataReady;
  sensor.getDataReadyStatus(dataReady);
  if (dataReady) {
    uint16_t co2;
    float temperature;
    float humidity;
    sensor.readMeasurement(co2, temperature, humidity);

    Serial.print("Luftfeuchtigkeit: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
  delay(1000);
}
