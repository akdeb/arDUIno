#include <driver/i2s.h>

#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define BUFFER_SIZE 1024

void i2sInit() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(SAMPLE_BITS),
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

void setup() {
    Serial.begin(115200);
    i2sInit();
    Serial.println("I2S Microphone Test");
}

void loop() {
    uint8_t i2sData[BUFFER_SIZE];
    size_t bytesRead;

    // Read audio data from I2S
    i2s_read(I2S_NUM_0, i2sData, BUFFER_SIZE, &bytesRead, portMAX_DELAY);

    // Print the raw data to the Serial Monitor
    for (size_t i = 0; i < bytesRead; i += 2) {
        int16_t sample = (i2sData[i + 1] << 8) | i2sData[i];
        Serial.println(sample);
    }

    delay(10); // Small delay to ensure the plotter updates smoothly
}
