/******************************************************************************
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
 *  Sensor: VIN<->3.3V  GND<->GND  SDA<->GPIO6  SCL<->GPIO7
 *  Liefert Neigungswinkel x/y/z in Grad. autoOffsets() beim Start: Sensor flach + ruhig halten.
 *  Waagrechte Lage: 0/0/0. Neigung wird positiv oder negativ ausgegeben.
 *****************************************************************************/

#ifndef LAGE_H
#define LAGE_H

#include <ICM20948_WE.h>
#include <math.h> // Benoetigt fuer atan2

#define ICM20948_ADDR 0x68
ICM20948_WE myIMU = ICM20948_WE(ICM20948_ADDR);

bool icmInitialized = false;
float lage_x = 0.0;
float lage_y = 0.0;

// aufgerufen in mc.ino
void setupLage()
{

  if (!myIMU.init())
  {
    Serial.println("ICM20948 antwortet nicht");
  }
  else
  {
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

// aufgerufen in mc.ino
void getLage()
{
  xyzFloat gValue;
  myIMU.readSensor();
  myIMU.getGValues(&gValue);

  // atan2 gibt nativ Werte von -180 bis +180 zurueck.
  // angleX: Vor/Zurueck-Neigung (Pitch)
  lage_x = atan2(gValue.y, gValue.z) * 180.0 / PI;

  // angleY: Links/Rechts-Neigung (Roll)
  lage_y = atan2(gValue.x, gValue.z) * 180.0 / PI;
}

#endif
