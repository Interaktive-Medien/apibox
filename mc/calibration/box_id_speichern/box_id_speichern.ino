#include <Preferences.h>

Preferences preferences;

// Trage hier deinen individuell ermittelten Kalibrierwert ein:
int boxid = 2;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("--- Starte Speichervorgang ---");

  // Namensraum "mikrofon" öffnen (false = Schreibzugriff)
  preferences.begin("sensorbox", false);

  // Wert unter dem Schlüssel "boxid" abspeichern
  preferences.putInt("boxid", boxid);

  // Speicher wieder schließen
  preferences.end();
}

void loop() {
  // Nichts zu tun
  delay(1000);
}