#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>
#include <ArduinoJson.h>

// Replace with your network credentials
// const char* ssid = "EE-P8CX8N";
// const char* password = "xd6UrFLd4kf9x4";

const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details
// const char* websocket_server = "echo.websocket.org";
// const uint16_t websocket_port = 443; // Secure WebSocket port
// const char* websocket_path = "/ws";

const char* websocket_server = "192.168.2.236";
const uint16_t websocket_port = 8000; // Secure WebSocket port
const char* websocket_path = "/starmoon";
const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";

// I2S pins for microphone
#define I2S_WS_MIC 15
#define I2S_SD_MIC 32
#define I2S_SCK_MIC 14

// I2S pins for speaker
#define I2S_WS_SPK 25
#define I2S_SD_SPK 33
#define I2S_SCK_SPK 26

// I2S configuration
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define BUFFER_SIZE 1024

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

bool isWebSocketConnected = false;
bool shouldReconnect = true;

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
            isWebSocketConnected = false;
            break;
        case WStype_CONNECTED:
            authMessage = createAuthTokenMessage(auth_token);
            Serial.println(authMessage);
            webSocket.sendTXT(authMessage);
            Serial.println("WebSocket Connected");
            isWebSocketConnected = true;
            break;
        case WStype_TEXT:
            Serial.printf("WebSocket Message: %s\n", payload);
            if (strcmp((char*)payload, "OFF") == 0) {
                webSocket.disconnect();
                isWebSocketConnected = false;
                shouldReconnect = false;
            }
            break;
        case WStype_BIN:
            // Play received audio data
            size_t bytes_written;
            i2s_write(I2S_NUM_1, payload, length, &bytes_written, portMAX_DELAY);
            break;
        default:
            Serial.println("Unknown WebSocket event");
            break;
    }
}

// Function to initialize the I2S for microphone
void i2sMicInit() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(SAMPLE_BITS),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_MIC,
        .ws_io_num = I2S_WS_MIC,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_MIC
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

// Function to initialize the I2S for speaker
void i2sSpkInit() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
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
        .bck_io_num = I2S_SCK_SPK,
        .ws_io_num = I2S_WS_SPK,
        .data_out_num = I2S_SD_SPK,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_1, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_1);
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

    // Initialize I2S for microphone and speaker
    i2sMicInit();
    i2sSpkInit();

    Serial.println("Setting up WebSocket...");
    webSocket.begin(websocket_server, websocket_port, websocket_path);
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket setup complete.");
}

void loop() {
    if (isWebSocketConnected || shouldReconnect) {
        webSocket.loop();
    }

    // Capture audio data from I2S microphone and send it over WebSocket
    if (isWebSocketConnected) {
        uint8_t i2sData[BUFFER_SIZE];
        size_t bytesRead;

        esp_err_t result = i2s_read(I2S_NUM_0, i2sData, BUFFER_SIZE, &bytesRead, 0);
        if (result == ESP_OK) {
            // Send data over WebSocket
            webSocket.sendBIN(i2sData, bytesRead);

            // Also send data to Serial for plotting
            for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++) {
                int16_t sample = ((int16_t*)i2sData)[i];
                Serial.println(sample);
            }
        } else {
            Serial.println("Error reading I2S data");
        }
    } else {
        Serial.println("WebSocket not connected");
        // Optionally, attempt to reconnect here
    }

    delay(10);  // Small delay to prevent tight looping
}
