/////////////////////////////////////////////////////////////////
/*
  Broadcasting Your Voice with ESP32-S3 & INMP441
  For More Information: https://youtu.be/qq2FRv0lCPw
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

/*
- Device
ESP32-S3 DevKit-C
https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html

- Required Library
Arduino ESP32: 2.0.9

Arduino Websockets: 0.5.3
https://github.com/gilmaimon/ArduinoWebsockets
*/

#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#define I2S_SD 14
#define I2S_WS 15
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0

#define bufferCnt 10
#define bufferLen 1024
int16_t sBuffer[bufferLen];

const char* ssid = "EE-P8CX8N";
const char* password = "xd6UrFLd4kf9x4";

// // const char* websocket_path = "wss://api.starmoon.app:8000/starmoon";
// const char* websocket_path = "wss://echo.websocket.org:443/";

// const char* websocket_server_host = "ws://api.starmoon.app";
// const uint16_t websocket_server_port = 8000;  // <WEBSOCKET_SERVER_PORT>

// const char* websocket_host = "echo.websocket.org";
// const uint16_t websocket_port = 443;

const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";
const char* ws_host = "192.168.1.241";
const uint16_t ws_port = 8000;
const char* ws_path = "/starmoon";

WebSocketsClient webSocket;
bool isWebSocketConnected = false;

// Function to create JSON message with the authentication token
String createAuthTokenMessage(const char* token) {
    StaticJsonDocument<200> doc;
    doc["token"] = token;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  static String authMessage; // Declare the String variable outside the switch statement
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected!");
      isWebSocketConnected = false;
      break;
    case WStype_CONNECTED:
      Serial.println("Connected!");
      authMessage = createAuthTokenMessage(auth_token);
            Serial.println(authMessage);
            webSocket.sendTXT(authMessage);
      isWebSocketConnected = true;
      break;
    case WStype_TEXT:
      Serial.printf("Got text: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.printf("Got binary length: %u\n", length);
      break;
    case WStype_PING:
      Serial.println("Got a PING!");
      break;
    case WStype_PONG:
      Serial.println("Got a PONG!");
      break;
  }
}

void i2s_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    //.sample_rate = 16000,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = bufferCnt,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);

  connectWiFi();
  connectWSServer();
  xTaskCreatePinnedToCore(micTask, "micTask", 10000, NULL, 1, NULL, 1);
}

void loop() {
  webSocket.loop();
}

void connectWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void connectWSServer() {
  webSocket.begin(ws_host, ws_port, ws_path);
  webSocket.onEvent(webSocketEvent);
  
  unsigned long connectionStart = millis();
  while (!isWebSocketConnected && millis() - connectionStart < 10000) {
    webSocket.loop();
    delay(10);
  }
  
  if (isWebSocketConnected) {
    Serial.println("WebSocket Connected!");
  } else {
    Serial.println("WebSocket Connection Failed!");
  }
}



void micTask(void* parameter) {

  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  size_t bytesIn = 0;
  while (1) {
    esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
    if (result == ESP_OK && isWebSocketConnected) {
webSocket.sendBIN((uint8_t*)sBuffer, bytesIn);    }
  }
      vTaskDelay(1); // Small delay to prevent watchdog issues
}