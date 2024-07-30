#include <WiFi.h>
#include <ArduinoWebsockets.h>

const char* ssid = "EE-P8CX8N";
const char* password = "xd6UrFLd4kf9x4";
const char* websockets_server = "wss://echo.websocket.org:443/"; //server address

using namespace websockets;

WebsocketsClient client;

void setup() {
    Serial.begin(115200);
    
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    
    Serial.println("Connected to WiFi");

    // Try to connect to Websockets server
    bool connected = client.connect(websockets_server);
    if(connected) {
        Serial.println("Connected to Websocket Server!");
        client.send("Hello Server");
    } else {
        Serial.println("Not Connected!");
    }
    
    // Setup Callbacks
    client.onMessage([](WebsocketsMessage message){
        Serial.print("Got Message: ");
        Serial.println(message.data());
    });
}

void loop() {
    if(client.available()) {
        client.poll();
    }
    delay(500);
}