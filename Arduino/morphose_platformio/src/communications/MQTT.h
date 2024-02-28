#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Arduino_JSON.h>
#include <VectorXf.h>
#include <WiFi.h>


namespace mqtt {

void initialize();

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connect();
void update();
void onLocation(int robot, char* data);
void onAnimation(char* data);

}   // namespace mqtt

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_
