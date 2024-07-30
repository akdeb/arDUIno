#include <WiFi.h>
#include <WebSocketsClient.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details
const char* websocket_server = "wss://api.starmoon.app";
// const uint16_t websocket_port = 80; // Ensure this is a port number
const char* websocket_path = "/starmoon";

// const char* websocket_server = "ws://192.168.2.236";
// const uint16_t websocket_port = 8000; // Ensure this is a port number
// const char* websocket_path = "/";

const char* token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";


// Create an instance of the WebSocket client
WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      webSocket.sendTXT("Hello from ESP32");
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket Message: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.println("WebSocket Binary Message");
      break;
  }
}

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // // Set up WebSocket
  // webSocket.begin(websocket_server, websocket_port, websocket_path);
  // webSocket.onEvent(webSocketEvent);
      
    // Set up WebSocket
    webSocket.begin(websocket_server, NULL, websocket_path);
    webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
