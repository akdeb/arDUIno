#include <WiFi.h>
#include <WebSocketsClient.h>

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

// WebSocket server details
const char* websocket_server = "echo.websocket.org";
const uint16_t websocket_port = 443; // Secure WebSocket port
const char* websocket_path = "/";

// Create an instance of the WebSocket client
WebSocketsClient webSocket;

String inputString = "";         // a string to hold incoming data
bool stringComplete = false;     // whether the string is complete

// Event handler for WebSocket events
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket Disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket Connected");
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

    // Set up WebSocket
    Serial.println("Setting up WebSocket...");
    webSocket.beginSSL(websocket_server, websocket_port, websocket_path);
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket setup complete.");
}

void loop() {
    webSocket.loop();
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
}

// SerialEvent occurs whenever a new data comes in the hardware serial RX. 
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
