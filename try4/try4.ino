#include "AudioTools.h"
#include "WiFi.h"
#include <WebSocketsClient.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details
const char* websocket_server = "192.168.2.236";
const uint16_t websocket_port = 8000;
const char* websocket_path = "/starmoon";

// Define the buffer size for I2S
#define BUFFER_SIZE 1024

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

// // Audio server (for local network streaming)
// AudioWAVServer localServer(ssid, password);

// I2S stream for capturing audio
I2SStream i2sStream;
ConverterFillLeftAndRight<int16_t> filler(RightIsEmpty); // fill both channels - or change to RightIsEmpty

bool isWebSocketConnected = false;

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      isWebSocketConnected = false;
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      isWebSocketConnected = true;
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket Message: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.println("WebSocket Binary Message");
      break;
    default:
      Serial.println("Unknown WebSocket event");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start I2S input with default configuration
  Serial.println("Starting I2S...");
  auto config = i2sStream.defaultConfig(RX_MODE);
  config.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT
  config.sample_rate = 44100;
  config.channels = 1;
  config.bits_per_sample = 16;
  i2sStream.begin(config);
  Serial.println("I2S started");

  // Initialize WebSocket
  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  // localServer.copy();

  if (isWebSocketConnected) {
    int bytesAvailable = i2sStream.available();
    if (bytesAvailable > 0) {
      uint8_t buffer[BUFFER_SIZE];
      int bytesRead = i2sStream.readBytes(buffer, min(bytesAvailable, BUFFER_SIZE));
      if (bytesRead > 0) {
        webSocket.sendBIN(buffer, bytesRead);
      }
    }
  }

  delay(10);  // Small delay to prevent tight looping
}