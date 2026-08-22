/**********************************************************************************************
* luftdruck.ino  (Test/Demo)
* BMP280 Luftdruckmessung
* Library: "Adafruit BMP280 Library" (+ Adafruit Unified Sensor)

 *  Anschluss:
 *  Sensor: VCC  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7

***********************************************************************************************/

#include <Wire.h>
#include <Adafruit_BMP280.h>

#define I2C_SDA 6
#define I2C_SCL 7

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
  float luftdruck = bmp.readPressure() / 100.0F;
  Serial.print(luftdruck);
  Serial.println(F(" hPa"));
  Serial.print(F("Berechnete Hoehe: "));
  delay(2000);
}
