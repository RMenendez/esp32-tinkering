# ESP32-S3 Bluetooth Low Energy (BTLE)

Control a LED connected to an ESP32 S3 from a mobile phone over BTLE (Bluetooth).
In the mobile use the Serial Bluetooth Terminal app downloaded from play store.

## Objective

Learn how to:

- configure the ESP32-C3 as a Bluetooth server
- advertise the server, a service with its properties for TX and RX
- control GPIO from text sent from Serial Bluetooth Terminal app. Using text like ON / OFF to turn on and off the LED

## Hardware

- ESP32-S3 Devkit C
- LED + resistor connected to GPIO4
- USB cable to connect to laptop

## Wiring

| ESP32-C3 | TO      |
|----------|---------|
| GPIO4    | LED     |
| GND      | 330 ohm resistor |


## Arduino IDE

Board:

ESP32S3 Dev Module

USB CDC On Boot: Enabled

Flash Size: 16MB


## How it works

The ESP32-S3 starts advertising as a BLE device

The phone connects to the ESP32 from Serial Bluetooth Terminal
selecting the BLE option (not Classic Bluetooth)

Using the mobile phone, once connecte, you can send a text string that is received by the ESP32 server, and can act depending on what is received. If "on" or "off" the LED is turned accordingly, if other text it is ignored. 

## What I learned

- ESP32 BTLE (Server, services, properties)
- Using Serial Bluetooth Terminal app in the mobile

## Problems / things to investigate

- Learn more about the concepts of server, service and properties in the BTLE connection