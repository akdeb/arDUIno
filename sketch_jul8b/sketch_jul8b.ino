const int analogPin = 34; // Analog output pin connected to D34 (GPIO34)
const int numSamples = 100; // Number of samples to capture

int samples[numSamples]; // Array to store the samples

void setup() {
  Serial.begin(115200); // Initialize serial communication
  pinMode(analogPin, INPUT); // Set the analog pin as input
}

void loop() {
  // Capture samples
  for (int i = 0; i < numSamples; i++) {
    samples[i] = analogRead(analogPin);
    delay(10); // Small delay to simulate sampling rate
  }

  // Send samples to Serial Monitor
  for (int i = 0; i < numSamples; i++) {
    Serial.println(samples[i]);
  }

  delay(1000); // Delay before capturing the next set of samples
}
