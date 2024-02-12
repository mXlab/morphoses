#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Arduino_JSON.h>
#include <VectorXf.h>
#include <WiFi.h>
#include <Globals.h>


// #define AVG_POSITION_TIME_WINDOW 0.2f

// MQTT settings.
#define MQTT_BROKER "192.168.0.200"
#define MQTT_BROKER_PORT 1883

namespace mqtt {






// Vec2f robotPositions[N_ROBOTS];
// Vec2f currPosition;

// TODO(Etienne): MOve to morphose
// Smoother avgPositionX(AVG_POSITION_TIME_WINDOW);
// Smoother avgPositionY(AVG_POSITION_TIME_WINDOW);
// Vec2f avgPosition;


void initialize();

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connect();

void update();





// TODO(Etienne): MOVE TO RESPECTIVE FILE
bool onMqttLocation(int robot, char* data);
//{
//   // Parse location.
//   JSONVar location = JSON.parse(data);
//   JSONVar position = location["position"];
//   if ((int)position["quality"] > 50) {

//     // Set current position.
//     Vec2f newPosition((float)double(position["x"]), (float)double(position["y"]));



//     robotPositions[robot].set(newPosition); // TODO: FIND OUT why storing robot position

//     // Assign.
//     if (robot+1 == robotId) {

//       currPosition.set(newPosition);
// //      bndl.add("/pos").add(currPosition.x).add(currPosition.y);
// //      currPosition += POSITION_ALPHA * ( newPosition - currPosition );
//     }

// //    bndl.add("/pos").add(robot+1).add(robotPositions[robot].x).add(robotPositions[robot].y);

//     sendOscBundle();
//   }

//}

void onMqttAnimation(char* data);
//{
//   // Parse location.
//   JSONVar animationData = JSON.parse(data);
//   JSONVar baseColor = animationData["base"];
//   JSONVar altColor  = animationData["alt"];

//   if (lockAnimationMutex()) {
//     prevAnimation.copyFrom(animation); // save animation

//     animation.setBaseColor(int(baseColor[0]), int(baseColor[1]), int(baseColor[2]));
//     animation.setAltColor (int(altColor[0]),  int(altColor[1]),  int(altColor[2]));
//     animation.setNoise(( float)  double(animationData["noise"]));
//     animation.setPeriod((float) double(animationData["period"]));
//     animation.setType( (AnimationType)int(animationData["type"]) );
//     animation.setRegion( (PixelRegion)int(animationData["region"]) );
// //    transitionTimer.duration(double(animationData["transition"]));

//     transitionTimer.start(); // start transition
//     unlockAnimationMutex();
//   }
//}










}   // namespace mqtt

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_MQTT_H_
