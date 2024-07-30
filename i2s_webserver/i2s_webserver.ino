#include "AudioTools.h"
#include "WiFi.h"

// Replace with your network credentials
const char* ssid = "launchlab";
const char* password = "LaunchLabRocks";

//AudioEncodedServer server(new WAVEncoder(), ssid, password);  
AudioWAVServer server(ssid, password); // the same as above

I2SStream i2sStream;    // Access I2S as stream
ConverterFillLeftAndRight<int16_t> filler(RightIsEmpty); // fill both channels - or change to RightIsEmpty

void setup(){
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // Connect to Wi-Fi
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

  // Start I2S input with default configuration
  Serial.println("Starting I2S...");
  auto config = i2sStream.defaultConfig(RX_MODE);
  config.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT
  config.sample_rate = 44100;
  config.channels = 1;
  config.bits_per_sample = 16;
  i2sStream.begin(config);
  Serial.println("I2S started");

  // Start data sink
  server.begin(i2sStream, config, &filler);
  Serial.println("Server started, you can access the stream at:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream.wav");
}

// Arduino loop  
void loop() {
  // Handle new connections
  server.copy();  
}
