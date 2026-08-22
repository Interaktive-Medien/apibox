/**********************************************************************************************
*  lage.ino  (Angepasst auf +/- 180 Grad)
*  ICM-20948 9-Degrees-of-Freedom Lagesensor — Neigungswinkel x/y/z.
*  Waagrechte Lage: 0/0/0. Neigung wird positiv oder negativ ausgegeben.
*  Library: ICM20948_WE von Wolfgang Ewald.
*  Sensor: VIN<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
***********************************************************************************************/

#include <Wire.h>
#include <ICM20948_WE.h>
#include <math.h>  // Benoetigt fuer atan2

#define ICM20948_ADDR 0x68

ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDR);

void setup() {
  Wire.begin(6, 7);
  Serial.begin(115200);
  while (!Serial) {}

  if (!myIMU.init()) {
    Serial.println("ICM20948 antwortet nicht");
  } else {
    Serial.println("ICM20948 ist verbunden");
  }

  Serial.println("Positioniere den ICM20948 flach und bewege ihn nicht - Kalibrierung...");
  delay(1000);

  // Die Kalibrierung sorgt dafuer, dass leichte Schieflagen der
  // Werkbank oder des Breadboards genullt werden.
  myIMU.autoOffsets();
  Serial.println("Fertig!");

  myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
  myIMU.setAccDLPF(ICM20948_DLPF_6);
  myIMU.setAccSampleRateDivider(10);
}

void loop() {
  xyzFloat gValue;
  myIMU.readSensor();
  myIMU.getGValues(&gValue);

  // atan2 gibt nativ Werte von -180 bis +180 zurueck.
  // angleX: Vor/Zurueck-Neigung (Pitch)
  float angleX = atan2(gValue.y, gValue.z) * 180.0 / PI;

  // angleY: Links/Rechts-Neigung (Roll)
  float angleY = atan2(gValue.x, gValue.z) * 180.0 / PI;

  // Ausgabe auf eine Nachkommastelle gerundet
  Serial.print("x:");
  Serial.print(angleX, 1);
  Serial.print(",y:");
  Serial.print(angleY, 1);

  delay(300);
}