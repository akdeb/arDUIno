#define BUTTON_PIN 18
#define LED_PIN 2  // The blue LED is connected to GPIO 2 on most ESP32 boards

void setup() {
  // Initialize the button pin as an input with an internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  
  // Turn off the LED initially
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Read the state of the button
  int buttonState = digitalRead(BUTTON_PIN);
  
  // If the button is pressed (LOW because of the internal pull-up resistor)
  if (buttonState == LOW) {
    // Turn on the LED
    digitalWrite(LED_PIN, HIGH);
  } else {
    // Turn off the LED
    digitalWrite(LED_PIN, LOW);
  }
}
