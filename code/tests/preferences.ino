/**********************************************************************************************
*  preferences.ino  (Test/Demo)
*  Persistentes Speichern/Laden im Flash via Preferences (putString / getString).
*  Original-Demo 14b_preferences_basics.ino — Referenz fuer Credentials/Kalibration.
*  Keine externe Library (Preferences ist eingebaut).
***********************************************************************************************/

#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(3000);

  preferences.begin("wifi", false);
  preferences.putString("ssid", "MeinNetzwerk");
  preferences.putString("password", "MeinPasswort");
  preferences.end();
}

void loop() {
  preferences.begin("wifi", true); // read-only

  String ssid = preferences.getString("ssid", "(leer)");
  String password = preferences.getString("password", "(leer)");

  preferences.end();

  Serial.println(ssid);
  Serial.println(password);
  delay(4000);
}
