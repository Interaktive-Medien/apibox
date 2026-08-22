/******************************************************************************************************
 * PROGRAMM 3: Messbetrieb mit aus Preferences geladenem Offset
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

float smoothedSPL = 0;
const float filterFactor = 0.1;

// Variable für den Offset (wird im setup() aus dem Speicher überschrieben)
float mic_db_offset = 120.0; 
Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. Gespeicherten Offset abrufen
  preferences.begin("mikrofon", true); // true = reiner Lesezugriff
  // Lese "dboffset". Falls er nicht existiert, verwende 120.0 als Fallback
  mic_db_offset = preferences.getFloat("dboffset", 120.0);
  preferences.end();

  Serial.print("Geladener mic_db_offset: ");
  Serial.println(mic_db_offset);

  // 2. I2S Mikrofon initialisieren
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
  
  Serial.println("Mikrofon bereit.");
}

void loop() {
  float dB = getDB();
  Serial.print("dB: ");
  Serial.println(dB);
}

float getDB() {
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
    float db = 20.0 * log10(rms + 1e-9);
    
    // Hier wird nun der geladene mic_db_offset addiert
    float spl = db + mic_db_offset;
    
    if (smoothedSPL == 0) smoothedSPL = spl;
    else smoothedSPL = (spl * filterFactor) + (smoothedSPL * (1.0 - filterFactor));
    return smoothedSPL;
  }
  delay(10);
  return smoothedSPL;
}