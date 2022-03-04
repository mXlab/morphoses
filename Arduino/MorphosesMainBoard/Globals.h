// Local (input) port is always 8000.
#define LOCAL_PORT 8000

// IP address of destination (PC).
#define DEST_IP_0 192
#define DEST_IP_1 168
#define DEST_IP_2 0
#define DEST_IP_3 100

#if ROBOT_ID == 1
#define DUAL_IMU true
#define NUM_PIXELS 8
#define PIXELS_TYPE (NEO_GRB + NEO_KHZ800)
//const uint8_t PIXEL_ZONES[][] = {
//  /* ALL */    { 0, 1, 2, 3, 4, 5, 6, 7, -1 },
//  /* TOP */    { 0, 1, 2, 3, 4, 5, -1 },
//  /* BOTTOM */ { 6, 7, -1 }
//};

#else // ROBOT_ID == 2
#define DUAL_IMU true
#define NUM_PIXELS 32
#define PIXELS_TYPE (NEO_BRGW + NEO_KHZ800)

//const uint8_t PIXEL_ZONES[3][] = {
//  /* ALL */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1 },
//  /* TOP */    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1 },
//  /* BOTTOM */ { 24, 25, 26, 27, 28, 29, 30, 31, -1 }
//};

#endif
