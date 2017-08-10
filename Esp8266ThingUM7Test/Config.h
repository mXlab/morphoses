#define AHRS true         // Set to false for basic data read
#define SerialDebug false  // Set to true to get Serial output for debugging
const bool OSCDebug = true; // debug-print OSC packet stuff
boolean sendOSC = true; // set to true to stream samples
#define useUdp true
#define AP_MODE false

const char *ssid = "iPhone de Rosalie Dumont Gagn\xC3\xA9";
const char *password = "3fz27iwkjmj1b";
#define DEST_IP_0 172
#define DEST_IP_1 20
#define DEST_IP_2 10
#define DEST_IP_3 6

//const char *ssid = "0Tatsnet"; // Pointer to the SSID (max 63 char)
//const char *password = "pochecon"; // for WPA2 min 8 char, for open use NULL
//#define DEST_IP_0 192
//#define DEST_IP_1 168
//#define DEST_IP_2 43
//#define DEST_IP_3 229

unsigned int localPort = 8765; // local port to listen for UDP packets
unsigned int destPort = 8767; // remote port to send UDP packets

