/******************************************************************************************************
 *  lautstaerke_kalibrieren.ino
 *  Einmalige Kalibration des INMP441-Mikrofons.
 *  Ermittelt den dB-Shift-Faktor und speichert ihn PERSISTENT in Preferences,
 *  damit mc.ino / lautstaerke.h ihn beim Start laden kann.
 *
 *  Keine Drittanbieter-Library (ESP-IDF I2S-Treiber).
 *  Anschluss:
 *    INMP441: VDD<->3.3V  GND<->GND  SD<->GPIO13  SCK<->GPIO2  WS<->GPIO23  L/R<->GND
 *
 *  Preferences:  Namespace "calib", Key "kf_lautst"
 *  (Kurzform der konzeptionellen Variable "kalibrationsfaktor_lautstaerke";
 *   NVS-Keys sind auf 15 Zeichen begrenzt.)
 *
 *  PRINZIP:
 *  Das Mikrofon liefert einen Roh-dB-Wert (db_raw = 20*log10(rms)), der ohne Shift negativ /
 *  unrealistisch ist. Wir brauchen einen Offset, der zum Rohwert addiert wird, damit ein
 *  sinnvoller Schallpegel (SPL) entsteht. Den Offset findet man durch Vergleich mit einer
 *  Referenz (z.B. Smartphone-dB-Messgeraet, App "Dezibel X").
 *
 *  ABLAUF (Serieller Monitor, 115200 Baud):
 *  1. Sketch flashen. Der Roh-dB-Wert (db_raw) wird laufend angezeigt.
 *  2. Gleichzeitig den realen Pegel mit dem Smartphone messen (z.B. 58 dB).
 *  3. Den gemessenen realen Wert im Serial Monitor eintippen (z.B. "58") + Enter.
 *  4. offset = realDB - db_raw wird berechnet und in Preferences gespeichert.
 ******************************************************************************************************/

#include <driver/i2s.h>
#include <math.h>
#include <Preferences.h>

#define I2S_WS          23
#define I2S_SD          13
#define I2S_SCK         2
#define I2S_PORT        I2S_NUM_0

#define SAMPLE_RATE     16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT

const int DMA_BUF_COUNT = 8;
const int DMA_BUF_LEN   = 1024;
const int BUFFER_SIZE   = 512;
int32_t samples[BUFFER_SIZE];

Preferences preferences;
float lastRawDB = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) != ESP_OK) {
    Serial.println("I2S Installation fehlgeschlagen!");
    return;
  }
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("=== INMP441 dB-Kalibration ===");
  Serial.println("Roh-dB wird angezeigt. Miss den realen Pegel mit dem Smartphone");
  Serial.println("und tippe den Wert (z.B. 58) hier ein + Enter.");
}

float getRawDB() {
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &samples, sizeof(samples), &bytesRead, portMAX_DELAY);
  if (result == ESP_OK && bytesRead > 0) {
    int samplesCount = bytesRead / 4;
    float sumSq = 0;
    for (int i = 0; i < samplesCount; i++) {
      int32_t val = samples[i] >> 8;
      float floatSample = (float)val / 8388608.0;
      sumSq += (floatSample * floatSample);
    }
    float rms = sqrt(sumSq / samplesCount);
    return 20.0 * log10(rms + 1e-9);   // Roh-dB (ohne Offset, typ. negativ)
  }
  return lastRawDB;
}

void loop() {
  lastRawDB = getRawDB();
  Serial.print("db_raw: ");
  Serial.println(lastRawDB);

  // Wenn der Nutzer einen realen dB-Wert eingibt: Offset berechnen und speichern
  if (Serial.available() > 0) {
    float realDB = Serial.parseFloat();
    if (realDB > 0) {
      float offset = realDB - lastRawDB;     // dieser Wert wird im Betrieb addiert
      preferences.begin("calib", false);
      preferences.putFloat("kf_lautst", offset);
      preferences.end();

      Serial.println("=== Kalibration abgeschlossen ===");
      Serial.print("Realer Pegel: "); Serial.print(realDB); Serial.println(" dB");
      Serial.print("db_raw: ");       Serial.print(lastRawDB); Serial.println(" dB");
      Serial.print("Offset (kf_lautst) = "); Serial.println(offset);
      Serial.println("Wert wurde in Preferences gespeichert.");
      Serial.println("Jetzt mc.ino flashen — der Faktor wird automatisch geladen.");
    }
  }
  delay(500);
}
