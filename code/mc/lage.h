/**********************************************************************************************
 *  lage.h
 *  ICM-20948 9-Degrees-of-Freedom Lagesensor via I2C
 *  Library: ICM20948_WE von Wolfgang Ewald
 *  basiert auf example: ICM-20948.ino (getAngles)
 *
 *  Anschluss:
 *  Sensor: VIN  <->  ESP32-C6: 3.3V
 *  Sensor: GND  <->  ESP32-C6: GND
 *  Sensor: SDA  <->  ESP32-C6: GPIO6
 *  Sensor: SCL  <->  ESP32-C6: GPIO7
 *
 *  Liefert Neigungswinkel x/y/z in Grad. autoOffsets() beim Start: Sensor flach + ruhig halten.
 **********************************************************************************************/

#ifndef LAGE_H
#define LAGE_H

#include <ICM20948_WE.h>

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

#define ICM20948_ADDR 0x68
ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDR);
bool icmInitialized = false;

// aufgerufen in mc.ino
void setupLage()
{
  if (!myIMU.init())
  {
    Serial.println("ICM20948 nicht gefunden!");
    icmInitialized = false;
    return;
  }
  icmInitialized = true;
  Serial.println("ICM20948 initialisiert — kalibriere (flach + ruhig halten)...");
  delay(1000);
  myIMU.autoOffsets();
  myIMU.setAccRange(ICM20948_ACC_RANGE_2G);
  myIMU.setAccDLPF(ICM20948_DLPF_6);
  myIMU.setAccSampleRateDivider(10);
  Serial.println("ICM20948 bereit.");
}

// aufgerufen in mc.ino
String getLage()
{
  float x = 0.0, y = 0.0, z = 0.0;
  if (icmInitialized)
  {
    xyzFloat angle;
    myIMU.readSensor();
    myIMU.getAngles(&angle);
    x = angle.x;
    y = angle.y;
    z = angle.z;
  }
  String wert = "{\"x\":" + String(x, 2) + ",\"y\":" + String(y, 2) + ",\"z\":" + String(z, 2) + "}";
  return createJsonResponse(wert, "°", "object<float,float,float>", "ICM-20948"); // in mc.ino
}

#endif
