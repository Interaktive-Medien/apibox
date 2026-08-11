/**********************************************************************************************
*  alkohol.ino  (Kalibrierte Messung in mg/L)
*  MQ-3 Alkohol-Gassensor — Messung der Atemalkoholkonzentration (AAK).
*  Library: "MQUnifiedsensor" by Miguel Califa
*  
*  ACHTUNG HARDWARE: Der Sensor wird mit 5V betrieben, der ESP32-C6 verträgt max 3,3V.
*  Zwingend einen Spannungsteiler (z.B. 10kOhm / 20kOhm) an A0 verwenden!
*  
*  Verkabelung:
*  Sensor VCC  <-> 5V (VIN am ESP)
*  Sensor GND  <-> GND
*  Sensor A0   <-> [Spannungsteiler] <-> GPIO5 am ESP32-C6
*  Sensor D0   <-> nicht angeschlossen
*  
*  Hinweis: Die Heizung benötigt 1-5 Min. Aufwärmzeit für stabile Messungen.
*  Fabrikneue Sensoren benötigen initial 24h "Burn-in" am Strom.
***********************************************************************************************/

// Include the library
#include <MQUnifiedsensor.h>

/************************ Hardware Related Macros ************************************/
#define         Board                   ("ESP32") // Board-Typ für die Library
#define         Pin                     5         // GPIO 5 am ESP32-C6
/*********************** Software Related Macros ************************************/
#define         Type                    ("MQ-3") // Sensortyp
#define         Voltage_Resolution      (5)      // 5V (gleicht den Spannungsteiler mathematisch aus)
#define         ADC_Bit_Resolution      (12)     // ESP32-C6 hat einen 12-Bit ADC (Werte 0-4095)
#define         RatioMQ3CleanAir        (60)     // RS / R0 = 60 in sauberer Luft
/***************************** Globals ***********************************************/
// Declare Sensor
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

void setup() {
  Serial.begin(115200); 

  // Konfiguration der mathematischen Parameter für ALKOHOL
  MQ3.setRegressionMethod(1); // _PPM =  a*ratio^b
  MQ3.setA(0.3934); 
  MQ3.setB(-1.504); 

  /***************************** MQ Init & Kalibrierung ******************************/ 
  MQ3.init(); 
  
  Serial.print("Kalibriere Sensor, bitte warten...");
  float calcR0 = 0;
  for(int i = 1; i <= 10; i++) {
    MQ3.update(); 
    calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
    Serial.print(".");
    delay(100);
  }
  MQ3.setR0(calcR0 / 10);
  Serial.println(" fertig!");
  
  // Hardware-Checks
  if(isinf(calcR0)) { Serial.println("Warnung: R0 ist unendlich. Verkabelung prüfen."); while(1); }
  if(calcR0 == 0) { Serial.println("Warnung: R0 ist null. Verkabelung prüfen."); while(1); }
  
  Serial.println("--- Starte Messung ---");
}

void loop() {
  // 1. ADC-Wert vom Pin einlesen
  MQ3.update(); 
  
  // 2. Wert berechnen (mg/L) und in einer lokalen Variable speichern
  float mgL = MQ3.readSensor(); 
  
  // 3. Saubere Ausgabe im Seriellen Monitor
  Serial.print("Alkoholkonzentration: ");
  Serial.print(mgL);
  Serial.println(" mg/L");
  
  // Messintervall (500 ms)
  delay(2000); 
}