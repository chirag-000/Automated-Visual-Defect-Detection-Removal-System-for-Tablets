#include <Arduino.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

void setup() {
  // Initialize the onboard LED pin
  pinMode(LED_BUILTIN, OUTPUT);

  // Start the serial communication
  Serial.begin(9600);
  myservo.attach(9, 600, 2300);  // (pin, min, max)
  // Provide a startup message
  Serial.println("STM32F401RE: Send '1' to turn ON the LED, '0' to turn it OFF");
}

void loop() {
  // Check if data is available in the serial buffer
  if (Serial.available() > 0) {
    // Read the incoming byte
    char receivedChar = Serial.read();

    // Take action based on the received value
    if (receivedChar == '1') {
      digitalWrite(LED_BUILTIN, HIGH); // Turn LED ON
      Serial.println("LED is ON");

      // Reverse the servo rotation sequence
      myservo.write(130);  // Move to 130 degrees
      delay(1000);

      myservo.write(0);    // Move back to 0 degrees
      delay(500);

    } else if (receivedChar == '0') {
      digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF
      Serial.println("LED is OFF");
    } else {
      Serial.println("Invalid input. Send '1' or '0'.");
    }
  }
}
