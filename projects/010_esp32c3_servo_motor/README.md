# Servo motors

If you need precise control over movement the clear choice is the servo motor.

Here I look at how a servo works, how to connect it to an ESP32 C3, and how to control its position using simple Arduino IDE code.

First with a microservo 9g or SG90.
Testing with libraries for servo in Arduino IDE
To learn the basics of servo control

The components inside a typical hobby servo motor are:
- A DC motor
- A gearbox
- A servo horn (the piece adjusted to the axis to trasmit movement)
- A potentiometer (moves with the rotation of the axis to generate a voltage that identifies position)
- A control unit. Receives the desired position signal from the microcontroller, normally encoded in the dutycycle of the PWM signal, and compares it with the signal from the potentiometer. If this error signal is not zero, the motor is moved to make it zero and so go to the desired position.


PWM works by sending a series of electrical pulses to the servo at regular intervals. For most hobby servos pulses are sent about 50 times per second (50 Hz). This means a new pulse arrives every 20 ms (1/50th of a second).

The control of the servo’s position isn’t how often we send pulses, but how long each pulse lasts. This duration is called the “pulse width” (or “duty cycle” of the PWM signal). 

- Short pulse around 1 ms or less -> move to 0 degrees (all the way to one side)
- Medium pulse of about 1.5 ms -> move the servo at 90 degrees (right in the middle)
- Longer pulse around 2 ms -> move the servo to 180 degrees (all the way to the other side)

For positions in between, use pulse widths that fall between these values. 
For example, if we want the servo at the 45-degree position (halfway between 0 and 90 degrees), we would use a pulse that’s about 1.25 ms long.

The exact pulse width range can vary slightly between different servo motor models, so check datasheet for each servo.




## Objective

Learn how to:

- Wire the ESP32-C3 to the microservo 9g. 
- Learn the basics of the control of the motor, with the use of the package for ESP32 #include <ESP32Servo.h> 
- Measure and check some of the parameters of the system, like the current used or the maximum torque.
- Control de movement using a potentiometer.

## Hardware

- ESP32-C3 super mini
- Servo motor: Tower Pro micro servo 9g SG90

To note that in the case of the SG90 there is no need for a motor driver circuit. The control signal PWM 5v comes directly from ESP32 C3. 

### Pinout
The servos usually have 3 wires ended in a standard three pin connector
- GND (usually black or brown)
- VCC (usually red color). Most hobby servos use between 4.8v and 6v. This is the case of the SG90
- Control (usually orange, yellow or white). This pin is to receive the PWM signal to control position


## Wiring

See diagram_wiring_servo_sg90.jpg

| ESP32-C3 | Servo          | Comments |
| -------- | -------------- | ---------- |
| GPIO0    | Control        |            |
| GND      | GND            | Common Ground |
|          | VCC            | External +5V power dedicated to servo |
|          |                |             |
| Power    |                | For tests, from USB (independent of motor power) |


For the potentiometer version see diagram_wiring_servo_sg90_pot.jpg
In this case we use pin GPIO1 to read the voltage value from the potentiometer.

| ESP32-C3 | Servo     | Potentiometer | Comments  |
| -------- | --------- | ------------- | ---------- |
| GPIO0    | Control   |               |  |
| GPIO1    |           | Common        |  |
| GND      | GND       | GND           | Common Ground |
|          | VCC       | VCC           | External +5V power dedicated to servo |
|          |           |               |  |
| Power    |           |               | For tests, from USB (independent of motor power) |



## Arduino IDE

Board:

ESP32C3 Dev Module

USB CDC On Boot: Enabled

Flash Size: 4MB


## How it works


### Direct version
The code in the ESP32-S3 010_sk_servo_sg90_sweep_esp32c3_20261026_ok.ino includes the following:

Tested with:
Servo motor: SG90 
Driver: No driver - direct connection of the control pin from ESP32 C3
Controller: ESP32 C3 super mini
 
Using #include <ESP32Servo.h> library
Define GPIO0 as the control pin for servo
Create instance of Servo class:  Servo myservo;
ESP32PWM::allocteTimer(0);   # Allow allocation of timers
myservo.setPeriodHert(50)  # Defines the frequency of the PWM signal

// attach the servoPin to the servo object and set the sweep range
// For SG90, 500 and 2400 might be more suitable
// different servos may require different min/max settings
myservo.attach(servoPin, 500, 2400);

int pos 0;             // defines pos variable and sets to 0
myservo.write(pos);    // tell servo to go to position in variable 'pos'


What current we see during movement?
And in stoped condition at 0 or 180?

### Potentiometer version

The code in the ESP32-S3 010b_sk_servo_sg90_potentiomenter_esp32c3_20261026_ok.ino.

It includes the potentiometer connected to GPIO1 and using:

valpot = analogRead(potPin);  // read the value of the potentiometer (0 to 4096 for 12-bit ADC)
valmapped = map(valpot, 0, 4096, 0, 180);  // scale it to use it with the servo (0 to 180)
myservo.write(valmapped);     // set the servo position according to the scaled value  



## What I learned

- Wiring of the servo SG900 to the ESP32, with and without potentiometer.
- Checked the servo SG90 180 working and turning from 0 to 180 degress at the change of the duty cycle of the PWM signal
- Usage of <ESP32Servo.h> library


## Problems / things to investigate

- For the SG90 test, if we increase the voltage of the motor to 6V, and keep the control signal direct from ESP32 (at 5v) could we have some issue?

- How can I measure the torque of the motor?
- How does a SG90 360 degrees servo works and what changes do we have to do to the code