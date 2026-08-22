/************************************************************
 * oled_display.ino  (Test/Demo)
 * SSD1306 OLED Display — Text anzeigen.
 * Original-Demo 07_OLED_SSD1306_Text.ino — Referenz fuer mc/display.h.
 * Library: Adafruit SSD1306 (+ Adafruit BusIO, Adafruit GFX).
 * Display: VDD<->3.3V  GND<->GND  SDA<->GPIO6  SCK<->GPIO7
 ************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 6
#define I2C_SCL 7
#define I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("Bun di !!");
  display.display();
}

void loop() {}
