# Installation instructions

## Install OSC tools

```sudo apt-get install pyliblo-utils```
This will install dump_osc and send_osc commandline tools.

## Connect to network
* Connect HOST to local wifi network.
* Use ```ifconfig``` to check the IP address of HOST computer.

## Test OSC communication (optional)

* If you can, use another computer (TEST) connected to the same network.
* On HOST computer run: ```dump_osc 12345```
* On TEST computer run: ```send_osc osc.udp://xxx.xxx.xxx.xxx:12345/ /hello```
* You should see ```/hello ,``` on the HOST

## Update Wifi configuration on main ESP8266 board

* From the root of the project: ```cd MorphosesMainBoard```
* Copy config file: ```cp MorphosesConfig.h.example MorphosesConfig.h```
* Open MorphosesMainBoard in Arduino and edit MorphosesConfig.h (see below)
* Upload to board

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

The relay pin needs to be put to HIGH in order to cut the current to the motors. This was done because when the ESP8266 is launched, there is a "spike" on pin 0 which would have triggered the motor initialization process (where it goes from side to side once). However, there is a way to fix it, by reprogramming the motor-control arduinos to wait (ie. adding a delay() in their setup()) so that they are not affected by the spike.

