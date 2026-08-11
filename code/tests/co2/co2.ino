/**********************************************************************************************
*  co2.ino  (Test/Demo)
*  SCD40 / SCD41 CO2 Sensor — Messe CO2-Konzentration, Temperatur und Luftfeuchtigkeit.
*  Demo-Code als Referenz fuer mc/co2.h.
*  Library: Sensirion I2C SCD4x (verlangt auch Sensirion Core)
*  Sensor: VCC<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
*
*  Typische Werte:
*    400 ppm = Frischluft, 800 = gut beluefteter Raum, 1000 = Buero, 2000 = schlechte Luft
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

    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm\t");
    Serial.print("Temperatur: ");
    Serial.print(temperature);
    Serial.print(" °C\t");
    Serial.print("Luftfeuchtigkeit: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
  delay(1000);
}
