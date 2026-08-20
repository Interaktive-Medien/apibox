/**********************************************************************************************
 *  lautstaerke.h
 *  INMP441 I2S MEMS-Mikrofon — Lautstaerke in dB
 *  Keine Drittanbieter-Library (ESP-IDF I2S-Treiber).
 *
 *  Anschluss:
 *  INMP441: VDD  <->  ESP32-C6: 3.3V
 *  INMP441: GND  <->  ESP32-C6: GND
 *  INMP441: SD   <->  ESP32-C6: GPIO13
 *  INMP441: SCK  <->  ESP32-C6: GPIO2
 *  INMP441: WS   <->  ESP32-C6: GPIO23
 *  INMP441: L/R  <->  ESP32-C6: GND
 *
 *  Kalibration: Der dB-Shift-Faktor wird persistent in Preferences gespeichert
 *  (Namespace "calib", Key "kf_lautst" — NVS-Keys max. 15 Zeichen, daher Kurzform
 *  der Variable "kalibrationsfaktor_lautstaerke"). Default 122.0.
 *  Ermittlung einmalig mit calibration/lautstaerke_kalibrieren.ino
 *  (Vergleich gegen ein Smartphone-dB-Messgeraet).
 **********************************************************************************************/

#ifndef LAUTSTAERKE_H
#define LAUTSTAERKE_H

#include <driver/i2s.h>
#include <math.h>
#include <Preferences.h>

// Forward Declaration (in mc.ino definiert)
// String createJsonResponse(String wert, String einheit, String datentyp, String sensor);

// Externer Zugriff auf Preferences (in wlan.h definiert)
extern Preferences preferences;

#define I2S_WS 23
#define I2S_SD 13
#define I2S_SCK 2
#define I2S_PORT I2S_NUM_0

#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT

const int LS_DMA_BUF_COUNT = 8;
const int LS_DMA_BUF_LEN = 1024;
const int LS_BUFFER_SIZE = 512;
int32_t ls_samples[LS_BUFFER_SIZE];

float smoothedSPL = 0;
const float ls_filterFactor = 0.1;

// dB-Shift aus Preferences (Variable: kalibrationsfaktor_lautstaerke)
float kalibrationsfaktor_lautstaerke = 122.0;

// aufgerufen in mc.ino
void setupLautstaerke()
{
  // Kalibrationsfaktor laden
  preferences.begin("calib", true); // in wlan.h
  kalibrationsfaktor_lautstaerke = preferences.getFloat("kf_lautst", 122.0);
  preferences.end();

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = LS_DMA_BUF_COUNT,
      .dma_buf_len = LS_DMA_BUF_LEN,
      .use_apll = false};

  i2s_pin_config_t pin_config = {
      .mck_io_num = I2S_PIN_NO_CHANGE,
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) != ESP_OK)
  {
    Serial.println("INMP441 I2S Installation fehlgeschlagen!");
    return;
  }
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  Serial.print("INMP441 initialisiert. dB-Shift = ");
  Serial.println(kalibrationsfaktor_lautstaerke);
}

// Misst aktuellen Schallpegel in dB (SPL)
float readDB()
{
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &ls_samples, sizeof(ls_samples), &bytesRead, portMAX_DELAY);

  if (result == ESP_OK && bytesRead > 0)
  {
    int samplesCount = bytesRead / 4;
    float sumSq = 0;

    for (int i = 0; i < samplesCount; i++)
    {
      int32_t val = ls_samples[i] >> 8;
      float floatSample = (float)val / 8388608.0;
      sumSq += (floatSample * floatSample);
    }

    float rms = sqrt(sumSq / samplesCount);
    float db = 20.0 * log10(rms + 1e-9);
    float spl = db + kalibrationsfaktor_lautstaerke; // Shift aus Preferences

    if (smoothedSPL == 0)
    {
      smoothedSPL = spl;
    }
    else
    {
      smoothedSPL = (spl * ls_filterFactor) + (smoothedSPL * (1.0 - ls_filterFactor));
    }
    return smoothedSPL;
  }
  return smoothedSPL;
}

// aufgerufen in mc.ino
String getLautstaerke()
{
  float dB = readDB();
  return createJsonResponse(String(dB, 2), "dB", "float", "INMP441"); // in mc.ino
}

#endif
