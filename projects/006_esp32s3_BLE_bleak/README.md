# ESP32-S3 Bluetooth and bleak

Control a LED connected to an ESP32 S3 from a laptop over BLE (Bluetooth).
In the laptop use the python package "bleak" for bluetooth LE.
The package "bleak" is installed in a uv .venv

## Objective

Learn how to:

- Use bleak package / library to create tests scripts to connect to the ESP32 S3 using BLE
- The ESP32 S3 is acting as a BLE server and can turn on / off a LED depending on the received commands
- Test the following processes:
  - Scan and detect BLE servers in range
  - Connect to a BLE server
  - Explore services and characteristics
  - Read value form a characteristic
  - Write to a characteristic, the one used to change LED status
- control GPIO from text sent from laptop, using text like ON / OFF to turn on and off the LED

## Hardware

- ESP32-S3 Devkit C
- LED + resistor connected to GPIO4
- USB power adapter to power the ESP32 S3 (not connected to laptop)

## Wiring

| ESP32-S3 | TO      |
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
