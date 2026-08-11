/**********************************************************************************************
 *  display.h
 *  SSD1306 OLED Display (128x64 Pixel) via I2C
 *  Zeigt Statusinformationen an (WLAN, IP, Meldungen).
 *  Library: Adafruit SSD1306 + Adafruit GFX + Adafruit BusIO
 *
 *  Anschluss:
 *  Display: VDD  <->  ESP32-C6: 3.3V
 *  Display: GND  <->  ESP32-C6: GND
 *  Display: SDA  <->  ESP32-C6: GPIO6
 *  Display: SCL  <->  ESP32-C6: GPIO7
 **********************************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// aufgerufen in mc.ino
void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("SSD1306 nicht gefunden!");
    return;
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
  Serial.println("Display initialisiert.");
}

// aufgerufen in mc.ino, wlan.h
void displayText(String text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void displayTwoLines(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 20);
  display.println(line2);
  display.display();
}

#endif
