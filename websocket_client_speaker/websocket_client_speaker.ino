#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

const char* websocket_server = "192.168.2.236";
// const char* websocket_server = "192.168.2.179";
const uint16_t websocket_port = 8000;
const char* websocket_path = "/starmoon";

// I2S configuration
#define I2S_WS 25
#define I2S_SD 22
#define I2S_SCK 26

i2s_config_t i2s_config = {
  .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX), // TX mode
  .sample_rate = 44100,
  .bits_per_sample = i2s_bits_per_sample_t(16),
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 8,
  .dma_buf_len = 64,
  .use_apll = false,
  .tx_desc_auto_clear = true
};

i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_SCK,
  .ws_io_num = I2S_WS,
  .data_out_num = I2S_SD,
  .data_in_num = I2S_PIN_NO_CHANGE
};

// WebSocket client
WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);

  // Setup WebSocket
  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      break;
    case WStype_TEXT:
      // Handle metadata if needed
      break;
    case WStype_BIN:
      // Write audio chunk to I2S
      size_t i2s_bytes_written;
      i2s_write(I2S_NUM_0, payload, length, &i2s_bytes_written, portMAX_DELAY);
      break;
    case WStype_ERROR:
      Serial.println("WebSocket Error");
      break;
    default:
            Serial.printf("Unknown WebSocket event. Type: %d, Payload: %s, Length: %d\n", type, payload, length);
            break;
  }
}
