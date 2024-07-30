const int buttonPin = 14; // GPIO14 connected to the push button
const int ledPin = 27;    // GPIO27 connected to the LED

int buttonState = 0;      // Variable to store the button state

void setup() {
  // Initialize serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Initialize LED pin as output
  pinMode(ledPin, OUTPUT);

  // Initialize button pin as input with internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Read the state of the push button
  buttonState = digitalRead(buttonPin);

  // Check if the button is pressed
  if (buttonState == LOW) { // Button is pressed
    // Turn the LED on
    digitalWrite(ledPin, HIGH);
    Serial.println("Button pressed, LED ON");
  } else { // Button is not pressed
    // Turn the LED off
    digitalWrite(ledPin, LOW);
    Serial.println("Button not pressed, LED OFF");
  }

  // Small delay to debounce the button
  delay(50);
}
