# ESP32-S3 BLE motor control

Control two motors connected to a DRV8833 motor control board through BLE (Bluetooth) talking to an ESP32 S3 board.
The system should allow a list of control commands (like forward x seconds, turn right 90 degrees, backward 1 second, etc)

## Objective

Learn how to:

- Connect two N20 motors to DRV8833 and then to ESP32 for controlling the motors
- Understand and test PWM with different values of frecuency and duty cycle, and draw some conclussions
- Learn about the DRV8833 motor driver
- Code to be able to send / run a sequence of commands including forward and backward during some seconds, and then some turns.
- Define and test the power requirements to have an autonomous battery powered robot with two wheels, able to move in the floor'


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

The ESP32-S3 starts advertising as a BLE device

The python scripts executed in laptop use laptop Bluetooth and bleak to do the following functions:

1-scan.py: 
Uses BleakScanner class to scan for BLE devices in range and print their address and name

2-connect_to_device.py
Uses BleakClient() class to connect to a specific device by its address, and prints if success

3-explore-services.py [MAC_ADDRESS]
- Connects to the device specified with [MAC_ADDRESS]
- Reads the services client.services (list)
- For each service, prints Service, Description, Handle
- Get list of characteristics of the service  "service.characteristics"
- For each characteristic prints UUID, Description, Handle, Properties
- If "read" in the properties, tries to read the value for the characteristic

4-read-characteristic.py
Uses BleakClient() class to connect to a specific device by its address, and then reads the value of a GATT characteristic in a service in that device. 
You have to provide the address of the device as first parameter.

```python
python 4-read-characteristic.py xx:xx:xx:xx:xx:xx
```

5-write-characteristic.py
Uses BleakClient() class to connect to a specific device by its address, and then writes a string, on/off in this case, to the value of a GATT characteristic in a service in that device. 
You have to provide the address of the device as first parameter.
The code includes the UUID of a Nordic UART RX characteristic, that is used in the ESP32 S3 code to turn a led on or off depending on the received value.

```pythoh
python 5-write-characteristic.py xx:xx:xx:xx:xx:xx
```


## What I learned

- Structure and relation between device(server), services, characteristics, properties, values
- All of them are components of GATT "General Attribute"
- How to use "bleak" python package to interact with a BLE device.


## Problems / things to investigate

- Learn and test the part of subscription and notifications
- Investigate if there ares some predefined UUIDs for specific services, or how to generate and use a custom UUID for a service I develop
