#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details
// const char* websocket_server = "echo.websocket.org";
// const uint16_t websocket_port = 443; // Secure WebSocket port
// const char* websocket_path = "/";

const char* auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Imp1bnJ1eGlvbmdAZ21haWwuY29tIiwidXNlcl9pZCI6IjAwNzljZWU5LTE4MjAtNDQ1Ni05MGE0LWU4YzI1MzcyZmUyOSIsImNyZWF0ZWRfdGltZSI6IjIwMjQtMDctMDhUMDA6MDA6MDAuMDAwWiJ9.tN8PhmPuiXAUKOagOlcfNtVzdZ1z--8H2HGd-zk6BGE";
const char* websocket_server = "192.168.2.236";
const uint16_t websocket_port = 8000; // Secure WebSocket port
const char* websocket_path = "/starmoon";

// Function to create JSON message with the authentication token
String createAuthTokenMessage(const char* token) {
    StaticJsonDocument<200> doc;
    doc["token"] = token;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// I2S pins
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

// I2S configuration
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define BUFFER_SIZE 1024

// Button pin
#define BUTTON_PIN 18

// LED pin
#define LED_PIN 2  // LED to indicate WebSocket connection

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

String inputString = "";         // a string to hold incoming data
bool stringComplete = false;     // whether the string is complete

// Variable to track WebSocket connection state
static bool isWebSocketConnected = false;
static bool shouldReconnect = true;

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

void i2sStop() {
    i2s_driver_uninstall(I2S_NUM_0); // Stop & destroy I2S driver
}

// Event handler for WebSocket events
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  static String authMessage; // Declare the String variable outside the switch statement
  
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket Disconnected");
            digitalWrite(LED_PIN, LOW); // Turn off LED
            isWebSocketConnected = false; // Update connection state
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket Connected");
            authMessage = createAuthTokenMessage(auth_token);
            Serial.println(authMessage);
            webSocket.sendTXT(authMessage);
            digitalWrite(LED_PIN, HIGH); // Turn on LED
            isWebSocketConnected = true; // Update connection state
            break;
        case WStype_TEXT:
            Serial.printf("WebSocket Message: %s\n", payload);
            if (strcmp((char*)payload, "OFF") == 0) {
                webSocket.disconnect();
                digitalWrite(LED_PIN, LOW); // Turn off the LED
                isWebSocketConnected = false; // Update connection state
                shouldReconnect = false; // Prevent automatic reconnection            
                }
            break;
        case WStype_BIN:
            Serial.println("WebSocket Binary Message");
            break;
        default:
            Serial.println("Unknown WebSocket event");
            break;
    }
}

// Function to initialize the button and LED
void initPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void setup() {
    Serial.begin(115200);
    inputString.reserve(200); // reserve 200 bytes for the inputString

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to Wi-Fi");

    // Initialize button and LED
    initPins();

    Serial.println("Setup complete.");
}

void loop() {
    // Check if the button is pressed
    if (digitalRead(BUTTON_PIN) == LOW && !isWebSocketConnected) {
        Serial.println("Button pressed. Connecting to WebSocket...");
        // Reset the shouldReconnect flag
        shouldReconnect = true;

        // Set up WebSocket
        webSocket.beginSSL(websocket_server, websocket_port, websocket_path);
        webSocket.onEvent(webSocketEvent);

        // Wait for the button to be released
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }

        Serial.println("Button released. WebSocket connection enabled.");
    }

    // Handle WebSocket
    if (isWebSocketConnected || shouldReconnect) {
        webSocket.loop();
    }

    // Handle Serial input
    if (Serial.available()) {
        serialEvent();
    }

    // Send message from Serial Monitor
    if (stringComplete) {
        // Clear the input buffer
        inputString.trim(); // Remove any trailing newlines
        webSocket.sendTXT(inputString);
        Serial.println("Message sent: " + inputString);
        inputString = "";
        stringComplete = false;
    }

    // If WebSocket is disconnected, stop I2S
    if (!isWebSocketConnected) {
        i2sStop();
    }
}

// SerialEvent occurs whenever new data comes in the hardware serial RX. 
void serialEvent() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
}
