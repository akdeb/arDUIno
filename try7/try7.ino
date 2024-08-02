#include <WiFi.h>
#include <WebSocketsClient.h>
#include "AudioTools.h"
#include <ArduinoJson.h>
#include <Audio.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details 
const char* websocket_server = "192.168.2.236";
// const char* websocket_server = "192.168.2.179";
const uint16_t websocket_port = 8000;
const char* websocket_path = "/starmoon";
const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";

// Define I2S connections
#define I2S_DOUT  22
#define I2S_BCLK  26
#define I2S_LRC   25

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

ConverterFillLeftAndRight<int16_t> filler(RightIsEmpty); // fill both channels - or change to RightIsEmpty

// Variable to track WebSocket connection state
static bool isWebSocketConnected = false;
static bool attemptWebsocketConnection = false;
static String authMessage; // Declare the String variable outside the switch statement

// Global variables to manage audio reception
bool isReceivingAudio = false;

#define AUDIO_BUFFER_SIZE 32768
uint8_t audioBuffer[AUDIO_BUFFER_SIZE];  // Adjust size as needed
size_t audioBufferIndex = 0;

void processAudioBuffer() {
    Serial.printf("Processing %d bytes of audio\n", audioBufferIndex);
    // Here you would typically send this to your I2S output
    // For example:
    // size_t bytes_written;
    // i2s_write(I2S_NUM_0, audioBuffer, audioBufferIndex, &bytes_written, portMAX_DELAY);
}

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

void startI2SOutput() {
    // Start I2S input with default configuration
        Serial.println("Starting I2S output...");
        auto i2sConfig = i2sStreamOutput.defaultConfig(TX_MODE);
    i2sConfig.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT
    i2sConfig.sample_rate = SAMPLE_RATE;
    i2sConfig.channels = 1;
    i2sConfig.bits_per_sample = 16;
  i2sConfig.pin_bck = I2S_BCLK;
  i2sConfig.pin_ws = I2S_LRC;
  i2sConfig.pin_data = I2S_DOUT;
  i2sStreamOutput.begin(i2sConfig);
}

void startI2SInput() {
          // Start I2S input with default configuration
        Serial.println("Starting I2S input...");
        auto config = i2sStreamInput.defaultConfig(RX_MODE);
        config.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT
        config.sample_rate = SAMPLE_RATE;
        config.channels = 1;
        config.bits_per_sample = 16;
        i2sStreamInput.begin(config);
        Serial.println("I2S input started");
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
    startI2SInput();
    startI2SOutput();
}

void appendToBuffer(uint8_t* payload, size_t length) {
if (length <= AUDIO_BUFFER_SIZE) {
                    memcpy(audioBuffer + audioBufferIndex, payload, length);
                    audioBufferIndex += length;
                    Serial.printf("Audio buffer now contains %d bytes\n", audioBufferIndex);
                } else {
                    Serial.println("Received chunk too large for buffer");
                }
}

void handleBinary(uint8_t * payload, size_t length) {
        i2sStreamOutput.write(payload, length); // Write the received binary data to I2S

      //         audio.copyToBuffer(payload, length);
      // audio.startSongFromBuffer();
  // Copy received data to the audio buffer
  // for (size_t i = 0; i < length; i++) {
  //   audioBuffer[audioBufferIndex++] = payload[i];
  //   if (audioBufferIndex >= AUDIO_BUFFER_SIZE) {
  //     // Buffer full, handle audio data here
  //     processAudioData(audioBuffer, AUDIO_BUFFER_SIZE);
  //     audioBufferIndex = 0; // Reset buffer index
  //   }
  // }
}

void processAudioData(uint8_t * data, int length) {
  // Process audio data here
  // This is where you would typically play the audio, save it to a file, etc.
  // i2sStreamOutput.write(data, length);


  Serial.println("Received audio data:");
  for (int i = 0; i < length; i++) {
    Serial.print((int)data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

// void handleBinary(uint8_t* payload, size_t length) {
//   Serial.print("Audio data chunk received: ");
//             for (size_t i = 0; i < length; i++) {
//                 Serial.print(payload[i], HEX);
//                 Serial.print(" ");
//             }
//             Serial.println();
// }

void handleText(uint8_t* payload, size_t length) {
    Serial.printf("WebSocket Message: %s\n", payload);

    if (strcmp((char*)payload, "OFF") == 0) {
        disconnectWebSocket();          
    } else {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.println("JSON parsing failed");
            return;
        }

        String msgType = doc["type"];
        if (msgType == "metadata") {
            // Process metadata
            Serial.println("Received metadata");
            // Reset audio buffer for new audio
            audioBufferIndex = 0;
            isReceivingAudio = true;
        }
        else if (msgType == "end_of_audio") {
            Serial.println("End of audio");
            // Process the complete audio buffer
            processAudioBuffer();
            isReceivingAudio = false;
        }
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


