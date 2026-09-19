#include <Preferences.h>

Preferences preferences;

// Trage hier deine Startwerte für die Sensorbox ein:
int boxid = 4;
int intervall_s = 20;        // Messintervall in Sekunden (z.B. 15)
float temp_offset = -10.0;     // Temperatur-Offset in °C (z.B. 5.0)

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("--- Starte Speichervorgang ---");

  // Namensraum "sensorbox" öffnen (false = Schreibzugriff)
  preferences.begin("sensorbox", false);

  // Werte abspeichern
  preferences.putInt("boxid", boxid);
  preferences.putInt("intervall_s", intervall_s);
  preferences.putFloat("temp_offset", temp_offset);

  // Speicher wieder schließen
  preferences.end();
}

void loop() {
  // Nichts zu tun
  delay(1000);
}