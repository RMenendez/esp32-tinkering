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



## Recommended version 1 structure

I’d use **two application tasks**, plus the BLE callbacks provided by the ESP32 BLE stack:

1. **Arduino `loop()` — command and BLE supervisor**
   - Receives copied BLE writes from a queue.
   - Parses and validates commands and their parameters.
   - Sends accepted movement requests to the motor controller.
   - Handles replies and status notifications from the motor controller, and sends them over BLE.
   - Does not perform motor timing or use blocking delays.

2. **`MotorControllerTask` — motor owner**
   - Is the **only** code that changes motor direction or PWM.
   - Runs a small state machine for idle or active timed movement.
   - Checks the movement deadline regularly and stops when it expires.
   - Handles urgent stop and disconnect notifications.

BLE callbacks stay short: copy the incoming write into a bounded message and enqueue it. They should not parse full commands, drive motors, wait, or call lengthy operations.

This keeps the application small while still isolating motor timing from BLE command handling. It also follows the existing test sketch’s PWM setup—10 kHz, 8-bit duty—although those values should be confirmed on the actual driver and motors. See the `motor test sketch` and the `BLE motor-control notes`.

### Queues and notifications

| Element | Purpose |
|---|---|
| `commandQueue` | Carries fixed-size BLE command messages from the write callback to `loop()`. |
| `motorRequestQueue` | Carries validated requests such as move, stop, or status query from `loop()` to `MotorControllerTask`. |
| `motorEventQueue` | Carries events and status snapshots back to `loop()`, such as movement started, movement stopped, or current status. |
| Task notification | Provides a fast stop path to the motor task for `STOP` and client disconnect events. |

Use fixed-size queue messages and explicit maximum lengths. In version 1, each GATT write can contain one complete command; reject oversized or malformed writes rather than silently truncating them. A queue-overflow policy should also be explicit: log/count the failure, do not report the command as accepted, and trigger a safe stop if an incoming command cannot be queued.

No mutex or semaphore should be needed if each resource has one owner: the motor task owns motor state and GPIO/PWM, while `loop()` owns command parsing and BLE responses. Queues transfer copies of messages between them.

### Command handling

The parser in `loop()` should accept only the specified forms:

- `FWD duration speed`
- `BACK duration speed`
- `LEFT duration speed`
- `RIGHT duration speed`
- `STOP`
- `STATUS`

It validates the exact token count, numeric values, duration limit of 10,000 ms, and speed range of 1–100. Invalid input gets an error response; it must not turn into a partial or default movement.

A movement becomes a typed request rather than a string. The motor task then maps it to motor directions and converts the speed percentage to an 8-bit PWM duty. The minimum effective duty may need calibration, as noted in the project’s `motor-control README`.

### Motor state machine

`MotorControllerTask` should start with both motors in a known safe output state. When it receives a movement request, it replaces the active movement, applies the new directions and PWM, and records a deadline. On each short periodic cycle—for example, every 5–10 ms—it checks for stop events and whether the deadline has expired. It should use a periodic FreeRTOS delay such as `vTaskDelayUntil()`, not `delay()`.

For turning in place, `LEFT` and `RIGHT` drive the motors in opposite directions. Which motor goes forward for a left turn depends on the physical motor orientation and wiring, so verify the direction mapping with the wheels raised. The existing test sketch’s `driveBrake()` sets both DRV8833 inputs low; that is generally a **coast** state, not active braking. Name and select the stop mode deliberately.

`STATUS` should be answered from a snapshot produced by the motor task. Movement-started and movement-stopped events can also flow back through `motorEventQueue` so `loop()` can notify the BLE client.

### Disconnects and interrupts

On client disconnect, request a stop immediately through the motor task’s notification path. The motor task should stop regardless of whether another movement request is waiting in its queue. BLE callbacks normally run in a task context, not an ISR; use the appropriate task-level FreeRTOS API for that callback context.

**No ISR is needed for version 1.** If a future driver fault output or encoder input requires interrupt handling, keep its ISR minimal: signal a task or record an event, and do the actual processing outside the ISR.

### Suggested source layout

Even if version 1 stays in one `.ino` file, organize it into these sections or functions:

- Configuration: GPIO pins, PWM settings, limits, and queue sizes.
- BLE setup and short callbacks.
- Command parsing and validation.
- Motor request/event message definitions.
- Motor-control state machine and output helpers.
- `setup()`: safe motor initialization, queues, BLE, and task creation.
- `loop()`: command processing and BLE response/notification handling.

This is a proposed structure only; no code has been implemented.



## Version 1 firmware

The first firmware proposal is implemented in
[`esp32s3_ble_motor_control.ino`](./esp32s3_ble_motor_control.ino). It uses the
Nordic UART Service UUIDs from the BLE example, so a BLE client writes commands
to RX and subscribes to TX notifications:

| Characteristic | UUID | Use |
| --- | --- | --- |
| RX | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write one command per GATT write |
| TX | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Receive responses and movement status notifications |

Supported commands:

| Command | Example |
| --- | --- |
| `FWD duration_ms speed_percent` | `FWD 2000 60` |
| `BACK duration_ms speed_percent` | `BACK 1000 40` |
| `LEFT duration_ms speed_percent` | `LEFT 500 50` |
| `RIGHT duration_ms speed_percent` | `RIGHT 500 50` |
| `STOP` | `STOP` |
| `STATUS` | `STATUS` |

Movement durations must be between 1 and 10,000 ms, and speed must be between
1 and 100 percent. A new movement replaces the active one. `LEFT` and `RIGHT`
turn in place by driving the two motors in opposite directions. Movement
automatically stops at its deadline; disconnecting the BLE client also requests
a stop. `STATUS` and start/stop events are returned as TX notifications, so the
client must enable notifications.

The sketch follows the existing motor test pin assignments (GPIO7/6 to IN1/IN2
and GPIO4/5 to IN3/IN4) and uses 10 kHz, 8-bit PWM with the Arduino-ESP32 3.x
`ledcAttach` API. The wiring table elsewhere in this README lists GPIO4/5 as
IN4/IN3, respectively, which is opposite to the test sketch and this firmware;
verify the actual wiring before applying motor power.
`MOTOR_A_INVERTED` and `MOTOR_B_INVERTED` can be adjusted
after checking the wheel directions with the robot raised. The turning mapping
assumes motor A is the left wheel and motor B is the right; change the
`LEFT`/`RIGHT` mappings if the motors are mounted on opposite sides. Both
inputs low is used as the stop/coast state. Validate the wiring and direction
mapping with the wheels raised before running the robot on the floor.

The implementation uses Arduino `loop()` for command parsing and BLE responses,
plus a FreeRTOS motor-controller task that exclusively owns motor outputs. BLE
callbacks enqueue bounded messages; no ISR or application-level mutex is used.
