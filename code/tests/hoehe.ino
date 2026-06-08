/**********************************************************************************************
* hoehe.ino  (Test/Demo)
* BMP280 Hoehenmessung (aus Luftdruck berechnet).
* Original-Demo 23_Luftdruck.ino — Referenz fuer mc/hoehe.h.
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
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!bmp.begin(0x76)) {
    Serial.println(F("Kein BMP280 gefunden!"));
    while (1) delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  float hoehe = bmp.readAltitude(SEALEVEL_HPA);
  Serial.print(F("Hoehe: "));
  Serial.print(hoehe);
  Serial.println(F(" m"));
  delay(2000);
}
