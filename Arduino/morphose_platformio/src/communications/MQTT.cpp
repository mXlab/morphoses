// #include "MQTT.h"

// #include <ArduinoLog.h>


// #include "communications/osc.h"
// #include "lights/Animation.h"
// #include "lights/Pixels.h"
// #include "Morphose.h"
// #include "Logger.h"



// namespace mqtt {
// // MQTT settings.
// const char* broker{"192.168.0.200"};
// const int brokerPort {1883};

// // RTLS IDs.
// static const char *ROBOT_RTLS_IDS[N_ROBOTS] = { "1a1e", "0f32", "5b26" };
// static char ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS][32];  // Internal use to keep full MQTT addresses.
// static char ROBOT_CUSTOM_MQTT_ADDRESS[32];

// // Create an ESP32 WiFiClient class to connect to the MQTT server.
// WiFiClient client;


//#if (ROBOT_ID ==1)
//const char* cid = "robot1";
//#elif (ROBOT_ID == 2)
//const char* cid = "robot2";
//#elif (ROBOT_ID == 3)
//const char* cid = "robot3";
//#endif

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
//Adafruit_MQTT_Client mqtt(&client, broker, brokerPort);
//Adafruit_MQTT_Client mqtt(&client, broker, brokerPort,cid,"","");


// // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
// //Adafruit_MQTT_Client mqtt(&client, broker, brokerPort);
// Adafruit_MQTT_Client mqtt(&client, broker, brokerPort,cid,"","");

// // Setup a feed called 'location' for subscribing to current location.
// Adafruit_MQTT_Subscribe* mqttRobotLocations[N_ROBOTS];
// Adafruit_MQTT_Subscribe* mqttAnimationData;

// Vec2f robotPositions[N_ROBOTS];
// Vec2f newPosition;


// void initialize() {
//   // Subscribe to RTLS robot location messages.
//   for (int i=0; i < N_ROBOTS; i++) {
//     // Create subscription.
//     sprintf(ROBOT_RTLS_MQTT_ADDRESS[i], "dwm/node/%s/uplink/location", ROBOT_RTLS_IDS[i]);
//     mqttRobotLocations[i] = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_RTLS_MQTT_ADDRESS[i]);

//     // Subscribe.
//     mqtt.subscribe(mqttRobotLocations[i]);
//   }

//   // Subscribe to own messages.
//   sprintf(ROBOT_CUSTOM_MQTT_ADDRESS, "morphoses/%s/animation", morphose::name);
//   mqttAnimationData = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_CUSTOM_MQTT_ADDRESS);
//   mqtt.subscribe(mqttAnimationData);

//   morphose::resetPosition();
// }


// void connect() {
//   osc::debug("Connecting to MQTT...");
//   int8_t error = mqtt.connect();

    
//     // buffer all robot positions
//     robotPositions[robot].set(newPosition);

//      if (robot+1 == morphose::id) {
//         morphose::setCurrentPosition(newPosition);
//      }
    

//     // TODO(Etienne): Verify with Sofian why whe send a bundle here. There was some commented code in old version here
//     //osc::sendBundle();
//   }
// }

// void onAnimation(char* data){
//   // Parse location.
//   JSONVar animationData = JSON.parse(data);
//   JSONVar _baseColor = animationData["base"];
//   JSONVar _altColor  = animationData["alt"];


//   if (animations::lockMutex()) {
//     animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
//     animations::currentAnimation().setBaseColor(int(_baseColor[0]), int(_baseColor[1]), int(_baseColor[2]));
//     animations::currentAnimation().setAltColor(int(_altColor[0]),  int(_altColor[1]),  int(_altColor[2]));
//     animations::currentAnimation().setNoise((float)  double(animationData["noise"]));
//     animations::currentAnimation().setPeriod((float) double(animationData["period"]));
//     animations::currentAnimation().setType((animations::AnimationType)int(animationData["type"]));
//     animations::currentAnimation().setRegion((pixels::Region)int(animationData["region"]) );
//     animations::beginTransition();  // start transition
//     animations::unlockMutex();
//   }
// }

// }   // namespace mqtt
