/******************************************************************************************************
 * magnet.ino  (Test/Demo)
 * GPS14-B Magnetsensor (Reed) — Tuer offen/zu. Gibt Zustand auf dem Serial Port aus.
 * Demo-Code (basiert auf 00_Button.ino) als Referenz fuer mc/magnet.h.
 * Keine Library noetig.
 * Sensor: Pin1<->GPIO11  Pin2<->GND   (INPUT_PULLUP: LOW = Magnet erkannt)
 ******************************************************************************************************/

const int magnetPin = 11;
int state = 0;
int prevState = -1;

void setup() {
  Serial.begin(115200);
  pinMode(magnetPin, INPUT_PULLUP);
}

void loop() {
  state = digitalRead(magnetPin);
  if (state == prevState) return;
  prevState = state;
  if (state == LOW) {
    Serial.println("Magnet erkannt (Tuer zu)");
  } else {
    Serial.println("kein Magnet (Tuer offen)");
  }
}
