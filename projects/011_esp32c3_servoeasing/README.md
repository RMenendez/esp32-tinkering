# ServoEasing library

Normally hobby servos always move at full speed and dont have means to reduce or control it.

One option to get this control, to allow for example to start and stop at lower speed, to make the movement smooth, is to divide the angle to rotate and do it in small steps, of different angle, controlling the delay between the steps. This is the base of easing.

There is a library in github from Arminjo that allows to do this and provides different curves of interpolation (linear, cuadratic, qubic, sine) and a lot of other features to control speed, synchrhonize movement for various servos, etc

The GitHub repo of this library is https://github.com/ArminJo/ServoEasing  (nov2026)

This project is about applying this library to a servo GeekServo Grey and experiment with it. This can be a good candidate to include in future projects like robot arm, or cranes.


## Objective

Learn how to:

- Control one or two servos using the servoEasing library
- Experiment with the different options, curves and ways to define speed
- Use ESP32 C3 super mini 
- Measure and confirm the values of start angle and end angle
- Test a two GeekServo Grey prototype of robot arm

## Hardware

- ESP32-C3 super mini
- Two Servo motors GeekServo 2Kg Grey
  - Positional 360 degrees with dual output axes
  - PWM Frequency 50Hz and pulse width from 500 to 2500 usec
  - Working voltage 3.3 to 6v  (Nominal 4.8)
  - Max Torque 1.8 kg-cm
  - Locked Rotor current 800mA
  - Operating speed  (0.14sec/60 degrees) measured at 4.8 v

In the tests done the servos have been powered with 5v and the control signal PWM 5v comes directly from ESP32 C3 without additional driver circuit. 

### Pinout

The GeekServo 2Kg 360 Grey uses the standard pinout. Has 3 wires ended in the standard three pin connector
- GND (usually black or brown)
- VCC (usually red color). Most hobby servos use between 4.8v and 6v. 
- Control (usually orange, yellow or white). This pin is to receive the PWM signal to control position


## Wiring


| ESP32-C3 | Servo          | Comments    |
| -------- | -------------- | ----------- |
| GPIO0    | Control        |             |
| GND      | GND            | Common Ground |
|          | VCC            | External +5V power dedicated to servo |
|          |                |             |
| Power    |                | For tests, from USB (independent of motor power) |



## Arduino IDE

Board:

ESP32C3 Dev Module

USB CDC On Boot: Enabled

Flash Size: 4MB


## How it works

The code in the ESP32-S3 011_sk_geekservo_servoeasing_esp32c3_20261112.ino includes the different tests with one servo:


## What I learned

- Wiring two servos GeekServo 360 Grey to the ESP32.
- Tested how to use the library <ServoEasing.hpp> and the use of the different curves in-out
- Using the attach to specify min and max values of pulse width, and the associated min and max angle

```cpp
  myservo2.attach(
    servoPin2,
    15,           // Initial position in degrees or microseconds
    SERVO_MIN_US, // 500 us for GeekServo grey (0 degrees)
    SERVO_MAX_US, // 2500 us for GeekServo grey (360 degrees)
    0,            // Min angle = 0
    360           // Max angle = 360
  );
```,
- Setting the easing type

```cpp
// Select cubic acceleration/deceleration
  //myservo2.setEasingType(EASE_CUBIC_IN_OUT);
  myservo2.setEasingType(EASE_SINE_IN_OUT);
```
- Setting the speed of the movement in two different ways (degrees per second / duration)

```cpp
  // Set speed in degrees per second for all movements
  // In this case you can specify the movement with startEaseTo(180);.
  // An alternativa is to specify startEaseTo(180, 60) to specify the 
  // speed in degrees per second in each of the movement commands
  // A different way of specifying speed is to use stareEaseToD(180, 1500)
  //  where D is for Duration and the second parm 1500 is duration in ms
  myservo2.setSpeed(ROTATIONAL_SPEED);  // setSpeed(60) to set 60 degrees per second
```

- Experiment with the management of two servos simultaneously and how to make them move non-blocking


## Problems / things to investigate

- How the speed in degrees / second affects the movement and what is the best for those servos.
The value I have used and seems appropiate is 70 - 90 degrees/second
- Test with other servos, for example SG90
- Measure the current drawn in the different situations
- See how to apply Bluetooth LE control to the arrangement