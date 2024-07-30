const int ledPin = 2; // LED connected to GPIO25 (D25)
const int analogPin = 34; // Analog output pin connected to D34 (GPIO34)
const int digitalPin = 33; // Digital output pin connected to D33 (GPIO33) (optional)

// #define LED 2 

int soundLevel;
int soundDetected;

void setup() {
  Serial.begin(115200);

  // Initialize LED pin as output
  pinMode(ledPin, OUTPUT);

  // Initialize analog pin as input
  pinMode(analogPin, INPUT);

  // Initialize digital pin as input
  pinMode(digitalPin, INPUT);
}

void loop() {
  // Read the analog value from the microphone sensor
  soundLevel = analogRead(analogPin);

  // Read the digital value from the microphone sensor
  soundDetected = digitalRead(digitalPin);

  // Print the analog value to the Serial Monitor
  Serial.print("Sound Level: ");
  Serial.println(soundLevel);

  // Print the digital value to the Serial Monitor
  if (soundDetected == HIGH) {
    Serial.println("Sound detected!");
    digitalWrite(ledPin, HIGH); // Turn on LED
  } else {
    Serial.println("No sound detected.");
    digitalWrite(ledPin, LOW); // Turn off LED
  }

  delay(500);

}
