/******************************************************************************************************
 * magnet.ino  (Test/Demo)
 * GPS14-B Magnetsensor (Reed) — Tuer offen/zu. Gibt Zustand auf dem Serial Port aus.
 * Demo-Code (basiert auf 00_Button.ino) als Referenz fuer mc/magnet.h.
 * Keine Library noetig.
 * Sensor: Pin1<->GPIO11  Pin2<->GND   (INPUT_PULLDOWN: HIGH = Magnet erkannt)
 ******************************************************************************************************/

const int magnetPin = 11;
int magnetstate = 0;
int prev_magnetstate = -1;

void setup() {
  Serial.begin(115200);
  pinMode(magnetPin, INPUT_PULLDOWN);
}

void loop() {
  magnetstate = digitalRead(magnetPin);
  if (magnetstate == prev_magnetstate) return;
  prev_magnetstate = magnetstate;
  if (magnetstate == 1) {
    Serial.println("Magnet erkannt");
  } else {
    Serial.println("kein Magnet erkannt");
  }
}
