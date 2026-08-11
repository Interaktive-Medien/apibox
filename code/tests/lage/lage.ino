/**********************************************************************************************
*  lage.ino  (Test/Demo)
*  ICM-20948 9-Degrees-of-Freedom Lagesensor — Neigungswinkel x/y/z.
*  Original-Demo ICM-20948.ino — Referenz fuer mc/lage.h.
*  Library: ICM20948_WE von Wolfgang Ewald.
*  Sensor: VIN<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
***********************************************************************************************/

#include <Wire.h>
#include <ICM20948_WE.h>
#define ICM20948_ADDR 0x68

ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDR);

void setup() {
  Wire.begin(6, 7);
  Serial.begin(115200);
  while (!Serial) {}

  if (!myIMU.init()) {
    Serial.println("ICM20948 does not respond");
  } else {
    Serial.println("ICM20948 is connected");
  }

  Serial.println("Position your ICM20948 flat and don't move it - calibrating...");
  delay(1000);
  myIMU.autoOffsets();
  Serial.println("Done!");

  myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
  myIMU.setAccDLPF(ICM20948_DLPF_6);
  myIMU.setAccSampleRateDivider(10);
}

void loop() {
  xyzFloat gValue;
  xyzFloat angle;
  myIMU.readSensor();
  myIMU.getGValues(&gValue);
  myIMU.getAngles(&angle);

  Serial.print("x:");
  Serial.print(angle.x);
  Serial.print(",y:");
  Serial.print(angle.y);
  Serial.print(",z:");
  Serial.println(angle.z);

  delay(300);
}
