#ifndef ROBOT_ID
#define ROBOT_ID 0
#error "Robot ID undefined"
#endif

// Network configuration.
#define WIFI_SSID "Morphoses"
#define WIFI_PASSWORD "BouleQuiRoule"

// Local (input) port is always 8000.
#define LOCAL_PORT  8000
#define REMOTE_PORT (8000 + 10*ROBOT_ID)

// IP address information for network.
#define NETWORK_IP_0 192
#define NETWORK_IP_1 168
#define NETWORK_IP_2 0

#define GATEWAY_IP_3 1 // Router
#define PC_IP_3      100 // Main PC controller
#define ROBOT_IP_3   (100 + 10*ROBOT_ID) // Robot