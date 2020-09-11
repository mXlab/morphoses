# Morphosis Project Code and Resources
This repository features scripts and resources for the ongoing Morphosis project.

## Subfolders

### Python

Contains python scripts for interacting with a Unity simulation and for conducting machine learning
experiments.

### Unity

Contains the Unity project for generating simulated data.

# Robot instructions

The robot is composed of several elements:

* A silicon-based shell
* A spherical shell
* A mechanical structure
* Two Arduinos - ESP8266 and ESP32
* Two inertial measurement units (IMU)
* One RTLS unit
* Two Li-Ion polymer batteries - 3.7v 1200mAh, with 2mm JST connectors
* One lead battery - 12v 1.3Ah 97x43x52mm 

## Placing / Removing the silicon-based shell

See the following video: TODO

## Opening / Closing the solid shell

The spherical shell consists of two interlocking half-spheres. At top of each lies a cap, which is locked to the corresponding half-sphere with four screws. One cap is glued with shell, while the other cap is attached to the mechanical structure.

To open the spherical shell, unscrew the two caps using an Allen key. Then, on the cap glued with shell, you may notice a small hole it its slice: put an Allen key inside the hole to reach the screw facing it and loosen the screw. Once it is done, you may carefully remove the cap with silicon from the mechanical structure. After this, flip the robot and grab the other cap so that all the mechanical structure's weight lies in your hand. Finally, take the mechanical structure carefully out of the spherical shell.

To close the spherical shell, do the same precedure backwards.

## Installing all batteries

There are two Li-Ion batteries. The first is placed on the mechanical structure; the second is placed on the cap glued with shell. In addition, there is one lead battery placed on the mechanical structure.

To install the first Li-Ion battery, connect the battery to the Arduino being careful to connect the red wires and the black wires together respectively. Then, use two elastic bands to attach the battery to the posts of the mechanical structure. Take care to tape the hanging wires so that they do not interfere with the movement of the robot.

To install the second Li-Ion battery, just connect the battery to the Arduino again, and put the battery inside the small leather pocket provided for this purpose. Take care to tape the hanging wires so that they do not interfere with the movement of the robot.

To install the lead battery, connect the black-ended wire at the plus terminal, and the white-ended wire at the minus terminal. Then, put the battery inside the counterweight taking care to center it well, and use the gold screw to attach it to the mechanical structure.

## Switching the robot on / off

There are three switches. The first enables to switch the IMU Arduino: it is on the cap glued with shell. The second enables to switch the Main Arduino as well as the RTLS unit: it is on the cap attached to the mechanical structure. The third enables to switch the robot's motors: it is also on the cap attached to the mechanical structure.

When you turn the first switch on, you should see red light emitted by the IMU, and blue light emitted by the Arduino.

When you turn the second switch on, you should see red light emitted by the IMU, blue light emitted by the Arduino, and red-yellow-green lights emitted by the RTLS unit.

Before turning the third switch on, be sure that you hold the mechanical structure horizontally so that the counterweight is pointing downwards and can swivel freely from left to right. When you turn it fully on, you should see a pendulum movement made by the counterweight, going all the way left, then all the way right, eventually stopping in the center.

## Charging all batteries

It is good practice to leave the batteries on charge overnight so that you can work all day the next day.

To charge the first Li-Ion battery, connect a USB charger to the USB plug under the shell. Be sure to leave the switch on while it charges.

To charge the second Li-Ion battery, connect a USB charger to the USB plug on the cap attached to the mechanical structure. Be sure to leave the switch on while it charges.

To charge the lead battery, connect a 12V charger to the power outlet on the cap attached to the mechanical structure. It is recommended to charge the lead battery at a different socket from the Li-Ion batteries.

# Wireless router instructions

The robot's displacement relies on a wireless router, which is shared by the two Arduinos, the RTLS, and your computer. It consists of:

* Any wireless router available on the market
* A modem providing an internet access

## Creating a local area network (LAN)

The local area network (LAN) is shared by the two Arduinos, the RTLS, and your computer.

To create a LAN, power the wireless router and connect it to the modem. Then, check if the LAN was created by displaying available wifi networks using your computer. Once it is done, connect your computer to the newly-created LAN (its name should be provided by the router manufacturer). Then, setup the newly-created LAN using the following service set identifier (SSID) and password by going to the wifi router web page (which should be provided by the router manufacturer):

* SSID: Morphoses
* Password: BouleQuiRoule

## Configuring the LAN

The LAN should be configured so that the two Arduinos, the computer, as well as the RTLS easily communicate together.

To start configuring the LAN, connect your computer to the LAN, and go to the wifi router page. Go to the "Attached devices" page: only your computer should be on that page. Then, switch the IMU Arduino on: you should see it added on the page. Repeat the procedure for the Main Arduino, as well as for the RTLS gateway: you should see each of them added on the page.

To finish confguring the LAN, go to "LAN setup" page. Edit all devices' IP addresses as indicated below (this will allow automatic communication between devices connected to the LAN):

* Main Arduino : 192.168.0.100
* IMU Arduino : 192.168.0.101
* Computer: 192.168.0.110
* RTLS gateway: 192.168.0.120

Additionally, you may also edit devices' name at your convenience to easily find them on the page.

# RTLS instructions

The Real-Time Location System (RTLS) uses Bluetooth to localise tags in space. We use a Decawave MDEK1001. It consists of:

* Four anchors - units framing the space
* One robot tag - the unit already mounted on the robot
* Additionnal tags - units allowing for location of other things/humans in space
* One gateway - RapsberryPi with one unit mounted on it

Additional information may be found in the following [documentation](https://www.digikey.ca/en/products/detail/decawave-limited/MDEK1001/7394534).

## Preparing the units

To prepare the anchors, select some of the RTLS units as anchors (4 is recommended for accuracy). Then, mount the anchors on the wall or on tripods (mounting them high will give better performance). Finally, power the anchors using USB batteries or USB power supplies.

To prepare the robot tag, power it by turning the robot's second switch on.

To prepare additionnal tags, power them using a power supply or USB batteries.

## Using the gateway

To use the gateway, power the RaspberryPi using a power supply. Then, connect your computer to the LAN, and go to http://192.168.0.120 (the local address that we set up for the gateway). You should be able to see all anchors and tags' location in real-time.

Importantly, the RTLS assigns each unit with an ID (e.g., 5a8e). You may note them down for further use in the Python scripts. For the latter, consider using small caps (even if the gateway indicates IDs with caps).

# Arduino instructions

The robot's angular position is measured by two Arduinos, which respectively process raw data coming from two inertial measurement units (IMUs). While the two Arduinos already embed all necessary code, you may need to rework it eventually (e.g., for debugging, or adding other sensing modules on the robot).

## Preparing Arduino software on your computer

Your computer may need specific programs to recognize the two types of Arduino boards (ESP32 and ESP8266) that are used on the robot.

- Install latest version of [Arduino](https://www.arduino.cc/en/Main/Software) and run it
- Go to "Tools > Library Manager" and install the following Arduino libraries: OSC (by A. Freed and Y. Mann), and Sparkfun BNO080 (by SparkFun Electronics)
- Go to "Preferences" and paste the following URLs in "Additional Boards Manager URLs": https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
- Go to "Tools > Boards > Board Manager" and install the following board managers: esp32 (by Espressif Systems), and esp8266 (by ESP8266 Community)
- Go to "Tools > Boards > ESP32 Arduino" and select "Adafruit ESP32 Feather" or "" to work on the IMU Arduino or the Main Arduino, respectively.

Install OSC tools: `sudo apt-get install pyliblo-utils`, or any of your preferred software/library to send/receive OSC messages.

## Connecting the Arduino to your computer

Connect the Arduino to your computer using a USB cable. Then, go to "Tools > Port" and select the newly-appeared serial port corresponding to the Arduino. If no new port appeared, you may need to install supplementary drivers on your computer. With Mac 10.11 and 10.14, the [CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers) were required. With other systems, you may browse the web to find the appropriate solution (unfortunately...).

## Testing OSC communication (optional)

- If you can, use another computer (TEST) connected to the same network.
- On HOST computer run: `dump_osc 12345`
- On TEST computer run: `send_osc osc.udp://xxx.xxx.xxx.xxx:12345/ /hello`
- You should see `/hello ,` on the HOST

## Updating Wifi configuration on Main ESP8266 board

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

If TensorFlow 1.5 cannot be found by pip, try installing it from TensorFlow packages: `pip install https://storage.googleapis.com/tensorflow/mac/cpu/tensorflow-1.5.0-py3-none-any.whl`

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
