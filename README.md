# Morphosis Project

This repository features scripts and resources for the ongoing Morphosis project by Sofian Audry and Rosalie D. Gagné.

## Local Network Configuration

```
 [[ MAIN ]]    [[ IMU ]]
  |     ^       |    ^
  |     |       |    |
(81x0)  |    (81x1)  |  
  |   (8000)    |  (8000)
  |     |       |    |
  v     |       v    |
{{ -- morphoses.py -- }}
           ^
           |
      (mqtt:1883)
           |
           v
{{ -- RTLS gateway -- }}
```

Default IP addresses (need to be setup on the router):

| Device           | IP            | Recv port | Send port |
| ---- | --- | :---: | :---: |
| Robot 1 MAIN Esp | 192.168.0.110 | 8000      | 8110      |
| Robot 1 IMU Esp  | 192.168.0.111 | 8000      | 8111      |
| Robot 2 MAIN Esp | 192.168.0.120 | 8000      | 8120      |
| Robot 2 IMU Esp  | 192.168.0.121 | 8000      | 8121      |
| Robot 3 MAIN Esp | 192.168.0.130 | 8000      | 8130      |
| Robot 3 IMU Esp  | 192.168.0.131 | 8000      | 8131      |
| Computer         | 192.168.0.100 | 81xx      | 8000      |
| RTLS Gateway     | 192.168.0.200 | 1883 (MQTT) | 1883 (MQTT) |

## Robot Calibration and TARE

### Calibration

When entering a new location it is best to calibrate the robot IMUs. In order to do so:
1. Begin calibration by sending OSC command ```/calibrate-begin```
2. Calibrate **accelerometer** by putting the robot in 6 different positions (as in the 6 faces of a "cube") and hold each position for ~2 seconds.
3. Calibrate **gyroscope** by putting the robot on the ground and let it stabilize, then wait ~2-3 seconds.
4. Calibrate **magnetometer** by rotating the device ~180° and back to the beginning position in each axis (pitch, roll, yaw) at a speed of about ~2 seconds for each axis.
5. Save calibration settings sending OSC command ```/calibrate-save```
 
### TARE

TARE needs to be performed on an already calibrated unit.
1. Put robot facing along the x axis of the room (according to the virtual coordinate system of the UWB positional system). Make sure the robot is stable and does not move.
2. Trigger tare by sending OSC command ```/tare-now```. You should see the Z euler angle become close to zero (0).
3. Save tare by sending OSC command ```/tare-save```.
4. (optional) Reboot the robot and move it a little bit so that it refreshes itself: it should then find back more-or-less its heading.

## Launching an Experiment

Launching an experiment requires to run the Python 3 script ```morphoses.py <behavior_file.yml>```


## Subfolders

### Python

Contains python scripts for interacting with a Unity simulation and for conducting machine learning
experiments.

### Machine Learning Scripts

All the ML scripts are in the Python/ml subfolder.

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
- Chrono

## Update Wifi configuration on main ESP32 board

- From the root of the project: `cd MorphosesMainBoard`
- Copy config file: `cp MorphosesConfig.h.example MorphosesConfig.h`
- Open MorphosesMainBoard in Arduino and edit MorphosesConfig.h (see below)
- Upload to board

Setup wifi AP here:

```
#define WIFI_SSID "Morphoses"
#define WIFI_PASSWORD "<password>"
```


# Mac installation (old)

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

