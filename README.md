# Morphosis Project

This repository features scripts and resources for the ongoing Morphosis project by Sofian Audry and Rosalie D. Gagné.

## Local Network Configuration

Default IP addresses (need to be setup on the router):

| Device            | IP            |
| ---- | --- |
| Main PC           | 192.168.0.100 |
| RTLS/MQTT Gateway | 192.168.0.200 |
| Robot 1 Esp32     | 192.168.0.110 |
| Robot 2 Esp32     | 192.168.0.120 |
| Robot 3 Esp32     | 192.168.0.130 |
| Visualizer PC     | 192.168.0.150 |
| Title display RPi | 192.168.0.161 |

MQTT Port: 1883

## Robot Calibration and TARE

### Calibration

When entering a new location it is best to calibrate the robot IMUs. In order to do so:
1. Begin calibration by sending MQTT command ```/calib begin```
2. Calibrate **accelerometer** by putting the robot in 6 different positions (as in the 6 faces of a "cube") and hold each position for ~2 seconds.
3. Calibrate **gyroscope** by putting the robot on the ground and let it stabilize, then wait ~2-3 seconds.
4. Calibrate **magnetometer** by rotating the device ~180° and back to the beginning position in each axis (pitch, roll, yaw) at a speed of about ~2 seconds for each axis.
5. Save calibration settings: ```/calib save```
6. Stop calibration settings: ```/calib stop```

### TARE

(Currently not working)

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
- Install the following Arduino libraries using the Library manager: Async MQTT, Sparkfun BNO080
- Follow instructions to install [ESP8266 Addon for Arduino](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon)
- Also install the [ESP32 Addon for Arduino](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
- Install latest version of [Processing](https://processing.org/download/)
- Install Processing library [OSCP5](http://www.sojamo.de/libraries/oscP5/)

## Connect to network

- Connect HOST to local wifi network.
- Use `ifconfig` to check the IP address of HOST computer.

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

