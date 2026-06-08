/**********************************************************************************************
* luftdruck.ino  (Test/Demo)
* BMP280 Luftdruck- und Hoehenmessung.
* Original-Demo 23_Luftdruck.ino — Referenz fuer mc/luftdruck.h.
* Library: "Adafruit BMP280 Library" (+ Adafruit Unified Sensor)
* Sensor: VCC<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
***********************************************************************************************/

#include <Wire.h>
#include <Adafruit_BMP280.h>

#define I2C_SDA 6
#define I2C_SCL 7

const float SEALEVEL_HPA = 1013.25;
Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);
  Serial.println(F("--- BMP280 Sensor-Test ---"));
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Fehler: Kein BMP280-Sensor gefunden! Verkabelung pruefen."));
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  float luftdruck = bmp.readPressure() / 100.0F;
  float hoehe = bmp.readAltitude(SEALEVEL_HPA);

  Serial.print(F("Luftdruck: "));
  Serial.print(luftdruck);
  Serial.println(F(" hPa"));
  Serial.print(F("Berechnete Hoehe: "));
  Serial.print(hoehe);
  Serial.println(F(" m"));
  Serial.println(F("-----------------------"));
  delay(2000);
}
