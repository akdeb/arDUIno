#include <WiFi.h>
#include <WebSocketsClient.h>
#include "AudioTools.h"
#include <ArduinoJson.h>
#include <driver/i2s.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details 
const char* websocket_server = "192.168.2.236";
// const char* websocket_server = "192.168.2.179";
const uint16_t websocket_port = 8000;
const char* websocket_path = "/starmoon";
const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";

// I2S pins for Audio Input (INMP441 microphone)
#define I2S_WS_IN 15    // LRCK
#define I2S_BCK_IN 14   // BCK
#define I2S_DATA_IN 32  // SD

// I2S pins for Audio Output (MAX98357A amplifier)
#define I2S_WS_OUT 25   // LRCK
#define I2S_BCK_OUT 26  // BCK
#define I2S_DATA_OUT 22 // DIN

// Button pin
#define BUTTON_PIN 18

// LED pin
#define LED_PIN 2  // LED to indicate WebSocket connection

// Define the buffer size for I2S
#define BUFFER_SIZE 1024
#define SAMPLE_RATE 16000

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
I2SStream i2sStreamInput;
I2SStream i2sStreamOutput;

// ConverterFillLeftAndRight<int16_t> filler(RightIsEmpty); // fill both channels - or change to RightIsEmpty

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
  // Start I2S for RX using AudioTools
  Serial.println("Starting I2S RX...");
  auto configRX = i2sStreamInput.defaultConfig(RX_MODE);
  configRX.port_no = I2S_NUM_0;  // Explicitly use I2S port 0 for input
  configRX.i2s_format = I2S_STD_FORMAT;
  configRX.sample_rate = SAMPLE_RATE;
  configRX.channels = 1;
  configRX.bits_per_sample = 16;
  // configRX.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
  configRX.pin_ws = I2S_WS_IN;
  configRX.pin_bck = I2S_BCK_IN;
  configRX.pin_data = I2S_DATA_IN;
  i2sStreamInput.begin(configRX);
  Serial.println("I2S RX started");

  // Start I2S for TX using AudioTools
  Serial.println("Starting I2S TX...");
  auto configTX = i2sStreamOutput.defaultConfig(TX_MODE);
  configTX.port_no = I2S_NUM_1;  // Explicitly use I2S port 1 for output
  configTX.i2s_format = I2S_STD_FORMAT;
  configTX.sample_rate = SAMPLE_RATE;
  // configTX.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  configTX.channels = 1; // Set to 2 for stereo
  configTX.bits_per_sample = 16;
  configTX.pin_ws = I2S_WS_OUT;
  configTX.pin_bck = I2S_BCK_OUT;
  configTX.pin_data = I2S_DATA_OUT;
  i2sStreamOutput.begin(configTX);
  Serial.println("I2S TX started");
}


// Function to handle WebSocket disconnection and deinitialization
void disconnectWebSocket() {
    webSocket.disconnect();
    i2sStreamInput.end();
    i2sStreamOutput.end();

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

void sendJsonToServer(const JsonDocument& doc) {
  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.sendTXT(jsonString);
}

void handleBinary(uint8_t * payload, size_t length) {
      // size_t i2s_bytes_written;
      // i2s_write(I2S_NUM_0, payload, length, &i2s_bytes_written, portMAX_DELAY);
      i2sStreamOutput.write(payload, length);
}

void handleText(uint8_t* payload, size_t length) {
    Serial.printf("WebSocket Message: %s\n", payload);

    if (strcmp((char*)payload, "OFF") == 0) {
        disconnectWebSocket();          
    } else {
    
    // DynamicJsonDocument doc(1024);
    // DeserializationError error = deserializeJson(doc, payload);
    // if (!error) {
    //   const char* msgType = doc["type"];
    //   if (strcmp(msgType, "metadata") == 0) {
    //         // Handle metadata if necessary
    //         Serial.println("Metadata received");
    //     } else if (strcmp(msgType, "end_of_audio") == 0) {
    //         // Handle end of audio signal
    //         StaticJsonDocument<200> doc;
    //           doc["speaker"] = "user";
    //         doc["is_replying"] = false;
    //         sendJsonToServer(doc);
    //     }
    // }
    }

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
            handleText(payload, length);
            break;
        case WStype_BIN:
            handleBinary(payload, length);
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
        int bytesAvailable = i2sStreamInput.available();
        if (bytesAvailable > 0) {
            uint8_t buffer[BUFFER_SIZE];
            int bytesRead = i2sStreamInput.readBytes(buffer, min(bytesAvailable, BUFFER_SIZE));
            if (bytesRead > 0) {
                webSocket.sendBIN(buffer, bytesRead);
            }
        }
    }

    delay(10);  // Small delay to prevent tight looping
}


