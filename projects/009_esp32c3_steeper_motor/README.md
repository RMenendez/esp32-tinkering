# Stepper motor test

Test a stepper motor.
First test with a 28BYJ-48 motor and a UNL2003 motor driver. 
Testing with both libraries in Arduino IDE (Stepper.h and AccelStepper.h)
To learn the basics for controlling those motors

The controller has been an ESP32 C3.

## Objective

Learn how to:

- Wiring the ESP32-C3 to the UNL2003 and to the motor B8BYJ-48 (see the included image) and test two possible groups of 4 GPIO pins to allow for two motor control.
- Learn the basics of the control of the motor, with the use of the two main packages 
  - Stepper.h
  - AccelStepper.h
- Measure and check some of the parameters of the system, like the limits in speed, the current used.

## Hardware

- ESP32-C3 super mini
- Motor driver UNL2003 board (for steeper motors)
- A 28BJY-48 stepper motor

## Wiring

See diagram_wiring_stepper_28BYJ.jpg

| ESP32-C3 | UNL2003        | MOTORS     | 
|----------|----------------|----------- |
| GPIO4    | IN1            |            |            
| GPIO5    | IN2            |            |
| GPIO6    | IN3            |            |
| GPIO7    | IN4            |            |
|          | Out connector  | Motor connector |


## Arduino IDE

Board:

ESP32C3 Dev Module

USB CDC On Boot: Enabled

Flash Size: 4MB


## How it works

The code in the ESP32-S3 includes the following:

Tested with:
Stepper motor: 28BYJ-48
Driver: ULN2003
Controller: ESP32 C3 super mini
Stepper.setSpeed(5) method sets speed to 5RPM
Tested from setSpeed(1) - with some vibration aprox 250 mA
Tested to setSpeed(17) - smooth move aprox 190 mA. Beyond 17 the motor stalls (don't move)
When stopped we see aprox 280 mA of current
The accuracy of setSpeed() in the RPMs is very high as measured with 4RPM and 8RPM and a chronometer (this has sense)


## What I learned

- ESP32 and UNL2003 wiring to control the motor.
- Tested that can use two different groups of four GPIO pins from ESP32 C3 so this controller could control two stepper motors (with two UNL2003 boards).
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
// Using pins GPIO4, GPIO1, GPIO3, GPIO0
// Tested in ESP32 C3 ok with pins 5,6,7,10 and also with 4,3,1,0
- Tested the effects of increasing the speed (and the limit about 17RPM where the motor stalls). 
- Also measured the current during operation in about 200mA, and see how it increases when very low speed or stopped to aprox 250mA
- Use of Stepper.h library from Arduino IDE

## Problems / things to investigate

- Learn more about other stepper motors and other motor driver boards
- Could the ESP32 S3 that has more GPIO pins support up to three stepper motors of the 28BJY-48 type?
- Add BLE control for speed and direction
