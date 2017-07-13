#ifndef MORPHOSES_CONFIG_
#define MORPHOSES_CONFIG_

#define MAIN_BOARD true

#define AHRS true         // Set to false for basic data read
#define SerialDebug false  // Set to true to get Serial output for debugging
const bool OSCDebug = true; // debug-print OSC packet stuff
boolean sendOSC = true; // set to true to stream samples
#define useUdp true
#define AP_MODE false

// ATTENTION: If these values are too low it seems to consume too much current causing the
// ESP8266 Thing to reset
//#define SEND_DATA_INTERVAL 500
#define SEND_DATA_INTERVAL 200
//#define SEND_DATA_INTERVAL 100

//const char *ssid = "ball"; // Pointer to the SSID (max 63 char)
//const char *password = "roller"; // for WPA2 min 8 char, for open use NULL
//
//#define DEST_IP_0 192
//#define DEST_IP_1 168
//#define DEST_IP_2 0
//#define DEST_IP_3 100

//const char *ssid = "Echo"; // Pointer to the SSID (max 63 char)
//const char *password = "010203040506070809"; // for WPA2 min 8 char, for open use NULL
//#define DEST_IP_0 192
//#define DEST_IP_1 168
//#define DEST_IP_2 1
//#define DEST_IP_3 103

//const char *ssid = "Patats"; // Pointer to the SSID (max 63 char)
//const char *password = "P0ch3c0n5"; // for WPA2 min 8 char, for open use NULL
//#define DEST_IP_0 192
//#define DEST_IP_1 168
//#define DEST_IP_2 0
//#define DEST_IP_3 100

const char *ssid = "0Tatsnet"; // Pointer to the SSID (max 63 char)
const char *password = "pochecon"; // for WPA2 min 8 char, for open use NULL
#define DEST_IP_0 192
#define DEST_IP_1 168
#define DEST_IP_2 43
#define DEST_IP_3 229


#if MAIN_BOARD
unsigned int localPort = 8765; // local port to listen for UDP packets
unsigned int destPort = 8766; // remote port to send UDP packets
#else
unsigned int localPort = 8765; // local port to listen for UDP packets
unsigned int destPort = 8767; // remote port to send UDP packets
#endif

// Pin definitions for ESP8266Thing
const int intPin = 4;  // incoming MPU9250 interrupt
const int redLed  = 5;  // red led is also green pin 5 on-board led
const int greenLed = 12;
const int blueLed = 13;
const int power = 0; // controls power to the rest of the ball
const byte MOTOR1_I2C_ADDRESS = 8; // i2c address of motor 1
const byte MOTOR2_I2C_ADDRESS = 16; // i2c address of motor 2
const byte MOTOR_SPEED = 1; // selector for motor speed
const byte MOTOR_POSITION = 2; // selector for motor encoder position
const byte MOTOR_RESET = 3; // selector for motor home

#define EEPROM_ADDRESS_MAG_BIAS 0
#define EEPROM_ADDRESS_MAG_SCALE (EEPROM_ADDRESS_MAG_BIAS + sizeof(float[3]))

#endif
