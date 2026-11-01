#include <ESP32Servo.h>

// create servo object to control a servo
Servo myservo;

// variable to store the servo position
int pos = 0;

// GPIO pin used to connect the servo
// Using GPIO0 in the ESP32 C3 super mini
int servoPin = 0;

void setup() {
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Standard 50hz servo
  myservo.setPeriodHertz(50);

  // attach the servoPin to the servo object and set the sweep range
  // For SG90, 500 and 2400 might be more suitable
  // different servos may require different min/max settings
  myservo.attach(servoPin, 500, 2400);
}

void loop() {

  // goes from 0 degrees to 180 degrees
  for (pos = 0; pos <= 180; pos += 1) {
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }

  // goes from 180 degrees to 0 degrees
  for (pos = 180; pos >= 0; pos -= 1) {
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
}