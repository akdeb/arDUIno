#include <WiFi.h>
#include <WebSocketsClient.h>
#include "AudioTools.h"
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details 
// const char* websocket_server = "192.168.2.236";
const char* websocket_server = "192.168.2.179";
const uint16_t websocket_port = 8000;
const char* websocket_path = "/starmoon";
const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";

// Button pin
#define BUTTON_PIN 18

// LED pin
#define LED_PIN 2  // LED to indicate WebSocket connection

// Define the buffer size for I2S
#define BUFFER_SIZE 1024

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

// Function to create JSON message with the authentication token
String createAuthTokenMessage(const char* token) {
    StaticJsonDocument<200> doc;
    doc["token"] = token;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// I2S stream for capturing audio
I2SStream i2sStream;
ConverterFillLeftAndRight<int16_t> filler(RightIsEmpty); // fill both channels - or change to RightIsEmpty

// Variable to track WebSocket connection state
static bool isWebSocketConnected = false;
static bool attemptWebsocketConnection = false;
static String authMessage; // Declare the String variable outside the switch statement

// Function to initialize the button and LED
void initPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void initWifi() {
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
}

void startI2S() {
          // Start I2S input with default configuration
        Serial.println("Starting I2S...");
        auto config = i2sStream.defaultConfig(RX_MODE);
        config.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT
        config.sample_rate = 16000;
        config.channels = 1;
        config.bits_per_sample = 16;
        i2sStream.begin(config);
        Serial.println("I2S started");
}

// Function to handle WebSocket disconnection and deinitialization
void disconnectWebSocket() {
    webSocket.disconnect();
    i2sStream.end();
    digitalWrite(LED_PIN, LOW); // Turn off LED
    isWebSocketConnected = false; // Update connection state
    attemptWebsocketConnection = false; // Prevent automatic reconnection
    Serial.println("WebSocket Disconnected and deinitialized");
}

void connectWebSocket() {
    Serial.println("WebSocket Connected");

    // auth step
    authMessage = createAuthTokenMessage(auth_token);
    Serial.println(authMessage);
    webSocket.sendTXT(authMessage);

    digitalWrite(LED_PIN, HIGH); // Turn on LED
    isWebSocketConnected = true; // Update connection state
    startI2S();
}

// Event handler for WebSocket events
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            disconnectWebSocket(); 
            break;
        case WStype_CONNECTED:
            connectWebSocket();
            break;
        case WStype_TEXT:
            Serial.printf("WebSocket Message: %s\n", payload);
            if (strcmp((char*)payload, "OFF") == 0) {
                disconnectWebSocket();          
            }
            break;
        case WStype_BIN:
            Serial.print("Received Binary Message of length: ");
            Serial.println(length);
            Serial.print("Data: ");
            for (size_t i = 0; i < length; i++) {
                Serial.printf("%02X ", payload[i]); // Print each byte in HEX
            }
            Serial.println(); // New line after printing all data
            break;
        default:
            Serial.printf("Unknown WebSocket event. Type: %d, Payload: %s, Length: %d\n", type, payload, length);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize button and LED
    initPins();
    initWifi();

    Serial.println("Setup complete.");
}

void loop() {
    // Serial.printf("isWebSocketConnected %d, attemptWebsocketConnection %d\n", isWebSocketConnected, attemptWebsocketConnection);
    // Check if the button is pressed
    if (digitalRead(BUTTON_PIN) == LOW && !isWebSocketConnected) {
        Serial.println("Button pressed. Connecting to WebSocket...");
        // Reset the shouldReconnect flag
        attemptWebsocketConnection = true;

        // Set up WebSocket
        webSocket.begin(websocket_server, websocket_port, websocket_path);
        webSocket.onEvent(webSocketEvent);

        // Wait for the button to be released
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }

        Serial.println("Button released. WebSocket connection enabled.");
    }

    // Handle WebSocket
    if (isWebSocketConnected || attemptWebsocketConnection) {
        webSocket.loop();
    }

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


