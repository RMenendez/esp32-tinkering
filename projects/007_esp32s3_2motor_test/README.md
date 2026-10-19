# ESP32-S3 Two motor test

Test two motors connected to a DRV8833 motor driver and to an ESP32 S3. The sketch should ramp up and down a PWM control signal, first in one direction and then in the other. First with motor A and then with motor B. It is a sketch to test the hardware, wiring, boards and motors.

## Objective

Learn how to:

- configure the ESP32-S3 as a motor controller using PWM pins
- use the PWM signal to drive a DRV8833 motor driver, powered with a 6v power signal
- control two different motors, and for each one, control direction (forward or backward) and speed (using the duty cycle of the PWM signal).

## Hardware

- ESP32-S3 Devkit C
- Motor driver DRV8833 board
- 2 x N20 motors + wheels

## Wiring

See diagram_wiring_two_motors_ESP32_S3.jpg

| ESP32-C3 | DRV8833 | MOTORS
|----------|---------|-----------
| GPIO4    | IN4     |
| GPIO5    | IN3     |
| GPIO6    | IN2     |
| GPIO7    | IN1     |
|          | OUT1    |  m1 +
|          | OUT2    |  m1 -
|          | OUT3    |  m2 +
|          | OUT4    |  m2 -


## Arduino IDE

Board:

ESP32S3 Dev Module

USB CDC On Boot: Enabled

Flash Size: 16MB


## How it works

The code in the ESP32-S3 includes the following:

- Assign variables to GPIO pins 
- Define frequency and resolution for PWM
- Forward and reverse directions depend on which pins receive PWM signal and which pins are at LOW level. There is a table of states.
- There is a part to Attach GPIO pins to PWM generator, specifying frequency and resolution
ledcAttach, ledcWrite(firstpin, speed), ledcWrite(secondpin, 0) 
- The loop makes a ramp up of duty_cycle for motor A, then ramps down and then it changes to do the same in motor B.


## What I learned

- ESP32 and DRV8833 wiring to control motors
- Define PWM by its Frequency (in KHz, and you have to consider the range from 500 to 1000Hz to 20KHz) and Resolution (the number of bits to divide the duty cycle. For example, with 8 bits you have 2**8 = 256 different levels of duty cycle.)
- Use of ledcAttach and ledcWrite


## Problems / things to investigate

- Learn more about motors
- Add BLE control for speed and direction
