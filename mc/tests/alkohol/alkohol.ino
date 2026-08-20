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
/*
 * MQ-3 Alkoholsensor Auswertung für ESP32
 * Sensor A0 ist über einen Spannungsteiler an GPIO 5 angeschlossen.
 */

#include <Arduino.h>

// --- Hardware Definitionen ---
constexpr uint8_t MQ3_PIN = 5;
constexpr int ADC_MAX = 4095;         // 12-Bit Auflösung beim ESP32
constexpr float V_REF = 3.3;          // Referenzspannung des ESP32

// --- Spannungsteiler Konfiguration ---
// Hier können die Widerstandswerte angepasst werden, falls der ADC clippt.
constexpr float R1 = 5600.0;          // Widerstand zwischen MQ-3 A0 und Knotenpunkt
constexpr float R2 = 12000.0;         // Widerstand zwischen Knotenpunkt und GND

// --- MQ-3 Sensor Parameter ---
// RL ist der Lastwiderstand auf dem Sensormodul (oft 200k Ohm, ggf. auf dem Board prüfen)
constexpr float RL = 200000.0;        
float R0 = 10000.0;                   // Wird in der setup() Routine kalibriert

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // ADC konfigurieren
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // Messbereich bis ca. 3.3V

    Serial.println("Starte MQ-3 Kalibrierung...");
    Serial.println("Bitte auf frische Luft achten und Sensor einlaufen lassen.");
    
    // Sensor aufwärmen und Basiswert ermitteln
    float sensorValueSum = 0;
    constexpr int CALIBRATION_SAMPLES = 100;
    
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        sensorValueSum += analogRead(MQ3_PIN);
        delay(50);
    }
    
    float avgAdc = sensorValueSum / CALIBRATION_SAMPLES;
    
    // R0 (Sensorwiderstand in sauberer Luft) berechnen
    // 1. Gemessene Spannung am ESP32-Pin
    float pinVoltage = (avgAdc / ADC_MAX) * V_REF;
    
    // 2. Ursprüngliche Spannung des Sensors (vor dem Spannungsteiler) rekonstruieren
    float sensorVoltage = pinVoltage * ((R1 + R2) / R2);
    
    // 3. Sensorwiderstand in sauberer Luft
    if(sensorVoltage > 0) {
        R0 = RL * ((5.0 - sensorVoltage) / sensorVoltage);
    }
    
    Serial.print("Kalibrierung abgeschlossen. R0 = ");
    Serial.print(R0);
    Serial.println(" Ohm");
}

void loop() {
    // --- 1. Messwert erfassen (Oversampling zur Glättung) ---
    long adcSum = 0;
    constexpr int SAMPLES = 20;
    
    for (int i = 0; i < SAMPLES; i++) {
        adcSum += analogRead(MQ3_PIN);
        delay(10);
    }
    
    float avgAdc = adcSum / (float)SAMPLES;

    // --- 2. ADC in Spannung und Rs (aktueller Sensorwiderstand) umrechnen ---
    float pinVoltage = (avgAdc / ADC_MAX) * V_REF;
    float sensorVoltage = pinVoltage * ((R1 + R2) / R2);
    
    float Rs = RL * ((5.0 - sensorVoltage) / sensorVoltage);
    
    // --- 3. Verhältnis berechnen ---
    float ratio = Rs / R0; // Verhältnis Rs/R0

    // --- 4. Näherung in mg/L ---
    // Der MQ-3 verhält sich logarithmisch. 
    // mg/L = a * (Rs/R0)^b (Werte a und b müssen aus dem Datenblatt abgeleitet werden)
    // Die folgenden Werte sind Standard-Näherungen für Alkohol beim MQ-3:
    float mgL = 0.4 * pow(ratio, -1.431);

    // --- 5. Ausgabe ---
    Serial.print("ADC: ");
    Serial.print(avgAdc, 0);
    Serial.print(" | V_Sensor: ");
    Serial.print(sensorVoltage, 2);
    Serial.print("V | Rs/R0: ");
    Serial.print(ratio, 2);
    Serial.print(" | Alkohol: ");
    Serial.print(mgL, 3);
    Serial.println(" mg/L");

    delay(1000);
}