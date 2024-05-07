#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_ASYNC_MQTT_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_ASYNC_MQTT_H_

#include <AsyncMqttClient.h>

#include <WiFi.h>

/**
 * @brief Reference 
 * 
 * Bug : no anchor showing  : https://forum.qorvo.com/t/dwm1001-on-raspberry-model-3b-cant-not-see-any-anchors-and-tages/12762/13
 * 
 */

namespace mqtt {

void initialize();
extern AsyncMqttClient client;
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void debug(const char * msg);
void sendTemperature(const char* msg);
void sendBatteryVoltage(float v);

namespace callbacks {
void handlePosition(int robot, char* data);
void handleAnimation(char* data);
void handleSteer(char* payload);
void handleSpeed(char* payload);
void handlePower(char* payload);
void handleGetData(char* payload);
void handleNav(char* payload);
void handleCalib(char* payload);
void handleStream(char* payload);
void handleReboot(char* payload);
void handleIdle(char* payload);

}


}   // namespace mqtt

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_ASYNC_MQTT_H_
