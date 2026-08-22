#include <Preferences.h>

Preferences preferences;

// Trage hier deinen individuell ermittelten Kalibrierwert ein:
int box_id = 1; 

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("--- Starte Speichervorgang ---");

  // Namensraum "mikrofon" öffnen (false = Schreibzugriff)
  preferences.begin("apibox", false); 
  
  // Wert unter dem Schlüssel "dboffset" abspeichern
  preferences.putInt("box_id", box_id);
  
  // Speicher wieder schließen
  preferences.end(); 
}

void loop() {
  // Nichts zu tun
  delay(1000);
}