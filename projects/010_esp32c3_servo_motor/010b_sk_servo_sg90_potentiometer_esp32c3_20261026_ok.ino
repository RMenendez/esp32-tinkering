#include <ESP32Servo.h>

// create servo object to control a servo
Servo myservo;

// GPIO pin used to connect the servo
// Recommended PWM pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPin = 0;

// GPIO pin used to connect the potentiometer
// Recommended ADC pins on the ESP32 include 0,2,4,12-15,32-39; 34-39 are recommended for analog input
int potPin = 1;

// Variable to store the value from the analog pin
int valpot;
int valmapped;

void setup() {

  Serial.begin(115200);
  delay(1000);

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
  valpot = analogRead(potPin);            // read the value of the potentiometer (0 to 4096 for 12-bit ADC)
  valmapped = map(valpot, 0, 4096, 0, 180);     // scale it to use it with the servo (0 to 180)
  myservo.write(valmapped);                  // set the servo position according to the scaled value
  Serial.printf("valpot: %d%% valmapped: %d\n", valpot, valmapped);
  delay(1000);                          // wait for the servo to reach the position
}