#ifndef MORPHOSE_GLOBAL_H
#define MORPHOSE_GLOBAL_H

// Local (input) port is always 8000.
#define LOCAL_PORT 8000

// Number of robots.
#define N_ROBOTS 3

// Battery energy thresholds.
// The low point for a NiMH (based on a cell at 1.2V) is 1.0V, so in our 10-cell battery pack this means that we have to stop at 10V.
// However we give ourselves a slightly higher target of 10.5V.
#define ENERGY_VOLTAGE_LOW      10.5f // Low energy
#define ENERGY_VOLTAGE_CRITICAL  9.8f // Critical energy

// IP address of destination (PC).
#define DEST_IP_0 192
#define DEST_IP_1 168
#define DEST_IP_2 0
#define DEST_IP_3 100



// RTLS IDs.
static const char *ROBOT_RTLS_IDS[N_ROBOTS] = { "1a1e", "0f32", "5b26" };
static char ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS][32]; // Internal use to keep full MQTT addresses.

static char ROBOT_CUSTOM_MQTT_ADDRESS[32];


// Neopixel settings.
#define NUM_PIXELS 32
#define NUM_PIXELS_PER_BLOCK 8
#define PIXELS_TYPE (NEO_GRBW + NEO_KHZ800)
//const uint8_t PIXEL_ZONES[3][] = {
//  /* ALL */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1 },
//  /* TOP */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1 },
//  /* BOTTOM */ { 24, 25, 26, 27, 28, 29, 30, 31, -1 }
//};

#endif
