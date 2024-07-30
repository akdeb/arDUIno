#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";
const char* websocket_server = "192.168.2.236";
const uint16_t websocket_port = 8000; // Secure WebSocket port
const char* websocket_path = "/starmoon";

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

// I2S configuration
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define BUFFER_SIZE 1024
#define BUTTON_PIN 18

bool buttonState = 0;  // Variable to store the button state

// Function to create JSON message with the authentication token
String createAuthTokenMessage(const char* token) {
    StaticJsonDocument<200> doc;
    doc["token"] = token;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// Function to initialize the button
void buttonInit() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

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

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to Wi-Fi");

    // Initialize I2S
    i2sInit();

    // Initialize button
    buttonInit();
      Serial.println("Setup complete.");
    // // Set up WebSocket
    // Serial.println("Setting up WebSocket...");
    // webSocket.begin(websocket_server, websocket_port, websocket_path);
    // webSocket.onEvent(webSocketEvent);
    // Serial.println("WebSocket setup complete.");
}

void loop() {
    buttonState = digitalRead(BUTTON_PIN);

    // Check if the button is pressed
    if (buttonState == LOW) {
      Serial.println("Button pressed. Connecting to WebSocket...");
      // Set up WebSocket
      webSocket.begin(websocket_server, websocket_port, websocket_path);
      webSocket.onEvent(webSocketEvent);

      // Wait for the button to be released
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }

      Serial.println("Button released. WebSocket connection enabled.");
    }

    webSocket.loop();

    // // Send binary audio data after authentication (for demonstration)
    // static bool audioSent = false;
    // if (webSocket.isConnected() && !audioSent) {
    //     webSocket.sendBIN(audioData, sizeof(audioData));
    //     Serial.println("Binary audio data sent");
    //     audioSent = true;
    // }
    uint8_t i2sData[BUFFER_SIZE];
    size_t bytesRead;
    i2s_read(I2S_NUM_0, i2sData, BUFFER_SIZE, &bytesRead, portMAX_DELAY);

    // Send audio data to WebSocket server
    if (webSocket.isConnected()) {
      webSocket.sendBIN(i2sData, bytesRead);
    }
}

