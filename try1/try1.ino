#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "AudioTools.h"

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";
const char* websocket_server = "192.168.2.236";
const uint16_t websocket_port = 8000; // Secure WebSocket port
const char* websocket_path = "/starmoon";

I2SStream i2sStream;
ConverterFillLeftAndRight<int16_t> filler(LeftIsEmpty); // fill both channels - or change to RightIsEmpty
WebSocketsClient webSocket;

// Buffer to store audio data before sending
const int BUFFER_SIZE = 1024;
uint8_t audioBuffer[BUFFER_SIZE];
int bufferIndex = 0;

// Function to create JSON message with the authentication token
String createAuthTokenMessage(const char* token) {
    StaticJsonDocument<200> doc;
    doc["token"] = token;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// Event handler for WebSocket events
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    static String authMessage; // Declare the String variable outside the switch statement

    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket Disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket Connected");
            authMessage = createAuthTokenMessage(auth_token);
            Serial.println(authMessage);
            webSocket.sendTXT(authMessage);
            // Send binary audio data to the WebSocket server upon connection
            // webSocket.sendBIN(audioData, sizeof(audioData));
            break;
        case WStype_TEXT:
            Serial.printf("WebSocket Message: %s\n", payload);
            break;
        case WStype_BIN:
            // Handle binary data received from the server
            for (size_t i = 0; i < length; i++) {
                Serial.printf("%02x ", payload[i]);
            }
            Serial.println();
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
        Serial.println(".");
    }
    Serial.println();
    Serial.println("Connected to Wi-Fi");

    // Set up WebSocket
    Serial.println("Setting up WebSocket...");
    webSocket.begin(websocket_server, websocket_port, websocket_path);
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket setup complete.");

    // Start I2S input
    Serial.println("Starting I2S...");
    auto config = i2sStream.defaultConfig(RX_MODE);
    // working well
    config.i2s_format = I2S_STD_FORMAT;
    config.sample_rate = 44100;  // INMP441 supports up to 44.1kHz
    config.channels = 1;         // INMP441 is mono
    config.bits_per_sample = 16; // INMP441 is a 24-bit ADC

    // config.pin_ws = 15;   // Adjust these pins according to your wiring
    // config.pin_bck = 14;
    // config.pin_data = 32;
    config.use_apll = true;  // Try with APLL for better clock stability
    i2sStream.begin(config);
    Serial.println("I2S started");
}

void loop() {
    webSocket.loop();

    // while (i2sStream.available()) {
    //     size_t bytesRead = i2sStream.readBytes(audioBuffer + bufferIndex, BUFFER_SIZE - bufferIndex);
    //     bufferIndex += bytesRead;

    //     // Send data if buffer is full or we have a significant amount of data
    //     if (bufferIndex >= BUFFER_SIZE || (bufferIndex > 0 && !i2sStream.available())) {
    //         webSocket.sendBIN(audioBuffer, bufferIndex);
    //         bufferIndex = 0;
    //     }
    // }

    // Optional: Add a small delay to prevent tight looping
    delay(1);
}
