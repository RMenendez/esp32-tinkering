/*
Tested with:
Stepper motor: 28BYJ-48
Driver: ULN2003
Controller: ESP32 C3 super mini
Stepper.setSpeed(5) method sets speed to 5RPM
Tested from setSpeed(1) - with some vibration aprox 250 mA
Tested to setSpeed(17) - smooth move aprox 190 mA. Beyond 17 the motor stalls (don't move)
When stopped we see aprox 280 mA of current
The accuracy of setSpeed() in the RPMs is very high (has sense)
measured with 4RPM and 8RPM and a chronometer
*/

//Includes the Arduino Stepper Library
// There is other library called AccelStepper with different syntax and options
#include <Stepper.h>

// Defines the number of steps per rotation
// The 28BYJ-48 with the reduction gear, needs 2048 steps for a complete turn
const int stepsPerRevolution = 2048;
const int stepsHalfRevolution = 1024;
const int stepsTwoRevolution = 4096;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
// Using pins GPIO4, GPIO1, GPIO3, GPIO0
// Tested in ESP32 C3 ok with pins 5,6,7,10 and also with 4,3,1,0
Stepper myStepper = Stepper(stepsPerRevolution, 5, 7, 6, 10);

void setup() {
  // Nothing to do (Stepper Library sets pins as outputs)
}

void loop() {
  // Rotate CW slowly at 5 RPM
  myStepper.setSpeed(10);
  myStepper.step(stepsPerRevolution);
  delay(1000);

  // Rotate CCW quickly at 10 RPM
  myStepper.setSpeed(10);
  myStepper.step(-stepsPerRevolution);
  delay(1000);

  // Rotate CW slowly at 5 RPM
////  myStepper.setSpeed(8);
////  myStepper.step(stepsTwoRevolution);
////  delay(2000);

}