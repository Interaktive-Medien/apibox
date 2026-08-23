/***************************************************
 *  gps.h
 *  GPS-Modul mit UART Schnittstelle
 *  Library: TinyGPSPlus by Mikal Hart z.B. V1.0.7
 *  Kommunikation zw. GPS-Modul und ESP32-C6 via UART
 *  Verbinde:
 *  GPS-Modul: TX  <-> ESP32-C6: GPIO19 (RX)
 *  GPS-Modul: RX  <-> ESP32-C6: GPIO18 (TX)
 *  GPS-Modul: Vcc <-> ESP32-C6: 3V3
 *  GPS-Modul: GND <-> ESP32-C6: GND
 *  sollte das GPS Modul noch weitere Pins haben, einfach ignorieren

 Wenn das GPS Modul sekündlich blau blinkt, wird ein GPS Signal empfangen.
 Die quadratische Keramikantenne muss mut dem Punkt nach oben positioniert werden. Glatte Seite unten.
 Ausgabe:

 Breitengrad: 0.000000
 Längengrad: 0.000000
 Höhe: 0.00 m
 Satelliten: 0
 Zeit/Datum: 2026-08-22 15:53:34
 ***************************************************/

#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Instanz der TinyGPS++ Bibliothek
TinyGPSPlus gps;

// HardwareSerial für GPS-Daten
HardwareSerial SerialGPS(1); // UART1 für das GPS-Modul
// Preferences preferences;

// Globale Variablen für GPS Daten
float latitude = 0;
float longitude = 0;
float altitude = 0;
String timeString = "";
int satellites = 0;

// WICHTIG: Der Timer muss zwingend hier oben (global) deklariert werden!
unsigned long lastPrintTime = 0;

void setupGPS()
{
  SerialGPS.begin(9600, SERIAL_8N1, 19, 18); // GPS: Baudrate 9600, RX=GPIO 19, TX=GPIO 18
  Serial.println("GPS-Modul wird initialisiert...");
}

// Funktion, um GPS-Daten auf der Konsole auszugeben
void displayGPSData()
{
  if (!gps.location.isValid())
  {

    preferences.begin("gps", true);                 // Namensraum "gps" öffnen (true = Lesezugriff)
    latitude = preferences.getFloat("latitude", 0); // der 2. Parameter gibt an, was angezeigt werden soll, wenn es keinen Wert im Speicher hat
    longitude = preferences.getFloat("longitude", 0);
    altitude = preferences.getFloat("altitude", 0);
    satellites = 0;
    timeString = preferences.getString("timeString", "");
    preferences.end();
  }
  else
  {
    // Neue, gültige Daten vom GPS auslesen
    if (gps.location.isUpdated() && gps.location.isValid())
    {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }

    if (gps.altitude.isUpdated())
    {
      altitude = gps.altitude.meters();
    }
    if (gps.satellites.isUpdated())
    {
      satellites = gps.satellites.value();
    }

    if (gps.date.isUpdated() && gps.time.isUpdated())
    {
      char timeBuf[32];
      snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
               gps.date.year(), gps.date.month(), gps.date.day(),
               gps.time.hour(), gps.time.minute(), gps.time.second());
      timeString = String(timeBuf);
    }

    // Werte in Preferences speichern (false = Schreibzugriff)
    // Info: putFloat/putInt schreiben automatisch nur dann in den Flash,
    // wenn sich der Wert auch wirklich geändert hat (schont den Speicherchip!).
    preferences.begin("gps", false);
    preferences.putFloat("latitude", latitude);
    preferences.putFloat("longitude", longitude);
    preferences.putFloat("altitude", altitude);
    preferences.putString("timeString", timeString);
    preferences.end();
  }

  // Ausgabe
  // Serial.printf("Breitengrad: %.6f\n", latitude);
  // Serial.printf("Längengrad: %.6f\n", longitude);
  // Serial.printf("Höhe: %.2f m\n", altitude);
  // Serial.printf("Satelliten: %d\n", satellites);
  // Serial.printf("Zeit/Datum: %s\n", timeString.c_str());
  // Serial.println("----------------------------");
}

// aufgerufen in mc.ino
void getGPS()
{
  // 1. Daten kontinuierlich in TinyGPS einspeisen (verhindert Buffer-Überlauf)
  while (SerialGPS.available() > 0)
  {
    char c = SerialGPS.read();
    gps.encode(c);
  }

  displayGPSData();
}

#endif
