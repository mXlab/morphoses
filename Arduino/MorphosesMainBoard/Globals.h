// Local (input) port is always 8000.
#define LOCAL_PORT 8000

// Number of robots.
#define N_ROBOTS 3

// IP address of destination (PC).
#define DEST_IP_0 192
#define DEST_IP_1 168
#define DEST_IP_2 0
#define DEST_IP_3 100

// MQTT settings.
#define MQTT_BROKER "192.168.0.200"
#define MQTT_BROKER_PORT 1883

// RTLS IDs.
const char *ROBOT_RTLS_IDS[N_ROBOTS] = { "1a1e", "0f32", "5b26" };
char ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS][32]; // Internal use to keep full MQTT addresses.

// Neopixel settings.
#define NUM_PIXELS 32
#define PIXELS_TYPE (NEO_BRGW + NEO_KHZ800)
//const uint8_t PIXEL_ZONES[3][] = {
//  /* ALL */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1 },
//  /* TOP */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1 },
//  /* BOTTOM */ { 24, 25, 26, 27, 28, 29, 30, 31, -1 }
//};
