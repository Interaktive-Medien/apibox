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
#include <WiFi.h>

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

extern JSONVar latestData;
extern int boxid;
int currentPage = 1;

void renderPage1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.printf("Sensorbox %d\n\n", boxid);
  display.println("kurzlink.ch/z36x");
  display.printf("?boxid=%d&sensor=[s]\n\n", boxid);
  display.println("Ersetze \"[s]\"");

  display.display();
}

void renderPage2() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("s =");

  display.printf("alkohol: %.2f mg/L\n", (double)latestData["alkohol"]);
  display.printf("bewegung: %d\n", (int)latestData["bewegung"]);
  display.printf("co2: %d ppm\n", (int)latestData["co2"]);
  display.printf("temperatur: %.1f \xF8"
                 "C\n",
                 (double)latestData["temperatur"]);
  display.printf("luftfeuchtigkeit:\n  %.1f %%\n",
                 (double)latestData["luftfeuchtigkeit"]);

  display.display();
}

void renderPage3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("s =");

  display.printf("distanz: %d mm\n", (int)latestData["distanz"]);
  display.printf("gewicht: %.1f g\n", (double)latestData["gewicht"]);
  display.printf("helligkeit: %.1f lux\n", (double)latestData["helligkeit"]);
  display.printf("lage_x: %.1f \xF8\n", (double)latestData["lage_x"]);
  display.printf("lage_y: %.1f \xF8\n", (double)latestData["lage_y"]);
  display.printf("luftdruck: %.1f hPa\n", (double)latestData["luftdruck"]);

  display.display();
}

void renderPage4() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("s =");

  if (latestData.hasOwnProperty("latitude")) {
    display.printf("- latitude: %.4f \xF8\n", (double)latestData["latitude"]);
    display.printf("- longitude: %.4f \xF8\n", (double)latestData["longitude"]);
    display.printf("- altitude: %.1f m\n", (double)latestData["altitude"]);
    display.printf("- gps_num_satellites:\n  %d\n",
                   (int)latestData["gps_num_satellites"]);
    display.printf("- gps_time: %s\n", (const char *)latestData["gps_time"]);
  } else {
    display.println("Warte auf Daten...");
  }
  display.display();
}

void renderPage5() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("s =");

  display.printf("- magnet: %d\n", (int)latestData["magnet"]);
  display.printf("- lautstaerke: %.1f dB\n", (double)latestData["lautstaerke"]);

  display.display();
}

void renderPage6() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.printf("Verbunden mit\n%s\n\n", WiFi.SSID().c_str());
  display.printf("IP Adresse:\n%s\n", WiFi.localIP().toString().c_str());

  display.display();
}

void renderCurrentPage() {
  switch (currentPage) {
  case 1:
    renderPage1();
    break;
  case 2:
    renderPage2();
    break;
  case 3:
    renderPage3();
    break;
  case 4:
    renderPage4();
    break;
  case 5:
    renderPage5();
    break;
  case 6:
    renderPage6();
    break;
  }
}

void nextDisplayPage() {
  currentPage++;
  if (currentPage > 6) {
    currentPage = 1;
  }
  renderCurrentPage();
}

void checkDisplayButton() {
  static int buttonState = LOW;
  static int lastButtonState = LOW;
  static unsigned long lastDebounceTime = 0;

  // PIN_BUTTON_DISPLAY wird in mc.ino definiert
  int reading = digitalRead(PIN_BUTTON_DISPLAY);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 50) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        nextDisplayPage();
      }
    }
  }
  lastButtonState = reading;
}

#endif
