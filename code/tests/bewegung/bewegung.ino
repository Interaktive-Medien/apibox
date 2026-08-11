/******************************************************************************************************
 * bewegung.ino  (Test/Demo)
 * SR602 PIR Bewegungssensor — gibt Bewegung (HIGH/LOW) auf dem Serial Port aus.
 * Demo-Code (basiert auf 00_Button.ino) als Referenz fuer mc/bewegung.h.
 * Keine Library noetig.
 * Sensor: VCC<->3.3V  GND<->GND  Data<->GPIO4
 ******************************************************************************************************/

const int pirPin = 4;
const int ledPin = BUILTIN_LED;
int state = 0;
int prevState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  state = digitalRead(pirPin);
  if (state == prevState) return;
  prevState = state;
  if (state == HIGH) {
    digitalWrite(ledPin, 1);
    Serial.println("Bewegung erkannt");
  } else {
    digitalWrite(ledPin, 0);
    Serial.println("keine Bewegung");
  }
}
