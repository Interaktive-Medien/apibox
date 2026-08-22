/******************************************************************************************************
 * button.ino  (Test/Demo)
 * Taster auslesen (z.B. Captive-Portal-Taster) und Wert auf Serial Port ausgeben.
 * Original-Demo 00_Button.ino — Referenz fuer den Taster in mc/captive_portal.h.
 * Sensor: Data<->GPIO20 (hier INPUT_PULLDOWN)
 ******************************************************************************************************/

const int buttonPin = 20;
const int ledPin = BUILTIN_LED;
int buttonState = 0;
int prev_buttonState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == prev_buttonState) return;
  prev_buttonState = buttonState;
  if (buttonState == 1) {
    digitalWrite(ledPin, 1);
    Serial.println(1);
  } else {
    digitalWrite(ledPin, 0);
    Serial.println(0);
  }
}
