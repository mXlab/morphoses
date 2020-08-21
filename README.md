# Morphosis Project Code and Resources
This repository features scripts and resources for the ongoing Morphosis project.

## Subfolders

### Python

Contains python scripts for interacting with a Unity simulation and for conducting machine learning
experiments.

### Unity

Contains the Unity project for generating simulated data.

# Arduino specific instructions

## Install tools

- Install OSC tools: `sudo apt-get install pyliblo-utils`
- Install latest version of [Arduino](https://www.arduino.cc/en/Main/Software)
- Install the following Arduino libraries using the Library manager: CNMAT OSC, Sparkfun BNO080
- Follow instructions to install [ESP8266 Addon for Arduino](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon)
- Also install the [ESP32 Addon for Arduino](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
- Install latest version of [Processing](https://processing.org/download/)
- Install Processing library [OSCP5](http://www.sojamo.de/libraries/oscP5/)

## Connect to network

- Connect HOST to local wifi network.
- Use `ifconfig` to check the IP address of HOST computer.

## Test OSC communication (optional)

- If you can, use another computer (TEST) connected to the same network.
- On HOST computer run: `dump_osc 12345`
- On TEST computer run: `send_osc osc.udp://xxx.xxx.xxx.xxx:12345/ /hello`
- You should see `/hello ,` on the HOST

## Arduino libraries

- OSC (CNMAT version by A. Freed & Y. Mann)
- Sparkfun BNO080 IMU

## Update Wifi configuration on main ESP8266 board

- From the root of the project: `cd MorphosesMainBoard`
- Copy config file: `cp MorphosesConfig.h.example MorphosesConfig.h`
- Open MorphosesMainBoard in Arduino and edit MorphosesConfig.h (see below)
- Upload to board

Setup wifi AP here:

```
const char *ssid = "ball"; // Pointer to the SSID (max 63 char)
const char *password = "roller"; // for WPA2 min 8 char, for open use NULL
```

Setup IP address of HOST here:

```
#define DEST_IP_0 192
#define DEST_IP_1 168
#define DEST_IP_2 0
#define DEST_IP_3 100
```

# Tech notes

## Relay pin

The relay pin needs to be put to HIGH in order to cut the current to the motors. This was done because when the ESP8266 is launched, there is a "spike" on pin 0 which would have triggered the motor initialization process (where it goes from side to side once). However, there is a way to fix it, by reprogramming the motor-control arduinos to wait (ie. adding a delay() in their setup()) so that they are not affected by the spike.

## Programming of microcontrollers

There are two kinds of microcontrollers on the robot:
 * ESP8266 Thing
 * Arduino Mini

Both types need to be programmed using a 3.3V FTDI cable. *Do NOT use a 5V FTDI cable/interface because the ESP8266 runs at 3.3V and you could break it.*

## Encoder Specs

Roller motor: 1365 impulsions per turn
Tilter motor: 1323 impulsions per turn

### ESP8266 Thing

Step 1: Connect the FTDI cable to the right input pins.

```
          |
GREEN --- | DTR
YELLOW -- | TX0
ORANGE -- | RX1
RED ----- | 3V3
BROWN --- | NC
BLACK --- | GND
          |
```

Step 2: Connect the DTR jumper.

Step 3: Select Boards > ESP8266 Thing and upload sketch.

If you have trouble with the ESP8266 please see [Troubleshooting the ESP8266](https://morphoseis.wordpress.com/2017/08/09/troubleshooting-the-esp8266/).

### Arduino Mini

Step 1: Connect the FTDI cable to the right input pins.

```
          -----------
GREEN --- | DTR
YELLOW -- | TX0
ORANGE -- | RX1
RED ----- | VCC
BROWN --- | GND
BLACK --- | GND
          -----------
```

Step 2: Select Boards > *Arduino Nano* and Processor > *ATmega328P (Old Bootloader)*

*Do NOT select Boards > Arduino Mini as it seems to be causing problems of the kind "avrdude: stk500_getsync() attempt ... of 10: not in sync: resp=0x0d" at upload.*

# Mac installation

Below are specific instructions to follow when installing the project on MacOS 10.11.6 (as the project was initially developed on Ubuntu).

## Unity

Resolve C# compatibility issue:

- Install Unity 2019.1.1f1 Personal
- Go to `Edit > Project Settings > Player > Other Settings > Configuration`
- Set Scripting Runtime Version to `.NET 4.x equivalent`

## Python

Resolve package installation issue:

- From the root of the project: `cd ./Python`
- Install Python virtual environment: `python3 -m venv Python`
- Launch environment: `source ./bin/activate`
- In a text editor, open `./Python/requirements.txt`
- Delete line `pkg-resources==0.0.0`
- Install requirements: `pip install -r requirements.txt`

Resolve TensorFlow issue with AVX instruction sets:

- From the root of the project: `cd ./Python`
- Launch Python virtual environment: `source ./bin/activate`
- Install TensorFlow 1.5: `pip install tensorflow==1.5`

# Machine Learning Scripts

All the ML scripts are in the Python/ml subfolder.

## Launching an experiment

Launching an experiment requires to run 2 programs:
- The Pythion script ```rl_curiosity.py```: runs the reinforcement learning process with tons of options
- The Python script ```robot_osc_bridge.py```: interfaces between ```RobotOscBridge``` and ```rl_curiosity.py``` to send and receive appropriately formatted OSC signals

```
 [[ MAIN ]]    [[ IMU ]]
  |     ^       |    ^
  |     |       |    |
(8766)  |    (8767)  |  
  |   (8765)    |  (8765)
  |     |       |    |
  V     |       V    |
{{ robot_osc_bridge.py }}
        |     ^
        |     |
     (7767)  (7765)
        |     |
        V     |
 {{ rl_curiosity.py }}
```

Default IP addresses (need to be setup on the router):

* MAIN Esp : 192.168.0.100
* IMU Esp : 192.168.0.101
* Computer: 192.168.0.110
* RTLS Gateway: 192.168.0.120
