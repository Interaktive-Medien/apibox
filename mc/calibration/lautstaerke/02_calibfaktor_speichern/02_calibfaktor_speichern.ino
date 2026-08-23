/******************************************************************************************************
 * PROGRAMM 2: Offset-Wert in Preferences speichern
 ******************************************************************************************************/

#include <Preferences.h>

Preferences preferences;

// Trage hier deinen individuell ermittelten Kalibrierwert ein:
float cal_mic_db_offset = 120.0; 

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("--- Starte Speichervorgang ---");

  // Namensraum "mikrofon" öffnen (false = Schreibzugriff)
  preferences.begin("mikrofon", false); 
  
  // Wert unter dem Schlüssel "dboffset" abspeichern
  preferences.putFloat("dboffset", cal_mic_db_offset);
  
  // Speicher wieder schließen
  preferences.end(); 

  Serial.print("Folgender mic_db_offset wurde dauerhaft gespeichert: ");
  Serial.println(cal_mic_db_offset);
  Serial.println("Du kannst nun Programm 3 aufspielen.");
}

void loop() {
  // Nichts zu tun
  delay(1000);
}