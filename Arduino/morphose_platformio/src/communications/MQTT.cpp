#include "MQTT.h"

#include <ArduinoLog.h>

#include "communications/osc.h"
#include "lights/Animation.h"
#include "lights/Pixels.h"
#include "Morphose.h"


namespace mqtt {
// MQTT settings.
const char* broker{"192.168.0.200"};
const int brokerPort {1883};

// RTLS IDs.
static const char *ROBOT_RTLS_IDS[N_ROBOTS] = { "1a1e", "0f32", "5b26" };
static char ROBOT_RTLS_MQTT_ADDRESS[N_ROBOTS][32];  // Internal use to keep full MQTT addresses.
static char ROBOT_CUSTOM_MQTT_ADDRESS[32];

// Create an ESP32 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, broker, brokerPort);

// Setup a feed called 'location' for subscribing to current location.
Adafruit_MQTT_Subscribe* mqttRobotLocations[N_ROBOTS];
Adafruit_MQTT_Subscribe* mqttAnimationData;

Vec2f robotPositions[N_ROBOTS];
Vec2f newPosition;


void initialize() {
  // Subscribe to RTLS robot location messages.
  for (int i=0; i < N_ROBOTS; i++) {
    // Create subscription.
    sprintf(ROBOT_RTLS_MQTT_ADDRESS[i], "dwm/node/%s/uplink/location", ROBOT_RTLS_IDS[i]);
    mqttRobotLocations[i] = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_RTLS_MQTT_ADDRESS[i]);

    // Subscribe.
    mqtt.subscribe(mqttRobotLocations[i]);
  }

  // Subscribe to own messages.
  sprintf(ROBOT_CUSTOM_MQTT_ADDRESS, "morphoses/%s/animation", morphose::name);
  mqttAnimationData = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_CUSTOM_MQTT_ADDRESS);
  mqtt.subscribe(mqttAnimationData);

  morphose::resetPosition();
}


void connect() {
  Log.infoln("Connecting to MQTT... ");
  int8_t error = mqtt.connect();
  if (error) {    // error detected
    char errorStr[64];
    strncpy_P(errorStr, (PGM_P)mqtt.connectErrorString(error), 64);
    Log.errorln(errorStr);


    // Send error
    osc::bundle.add("/error").add("mqtt-connect").add(errorStr);
    osc::sendBundle();
    mqtt.disconnect();
    
  } else {
      osc::bundle.add("/ready").add("mqtt-connect");
      osc::sendBundle();
    Serial.println("MQTT Connected!");
  }
}

void update() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.

    // Stop if already connected.
  if (!mqtt.connected()) {
      connect();
  }

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(10)) {
    if (subscription == mqttAnimationData) {
      Log.infoln("Animation updated");
     onAnimation((char *)mqttAnimationData->lastread);
    } else {
      for (int i=0; i < N_ROBOTS; i++) {
        if (subscription == mqttRobotLocations[i]) {
            //Log.infoln("Location %d updated", i+1);
            onLocation(i, (char *)mqttRobotLocations[i]->lastread);
            break;
        }
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

void onLocation(int robot, char* data) {
  // Parse location.
  JSONVar location = JSON.parse(data);
  JSONVar position = location["position"];
  if ((int)position["quality"] > 50) {
    // Set current position.
    newPosition = Vec2f((float)double(position["x"]), (float)double(position["y"]));  // weird way to do this
    
    // buffer all robot positions
    robotPositions[robot].set(newPosition);

     if (robot+1 == morphose::id) {
        morphose::setCurrentPosition(newPosition);
     }
    

    // TODO(Etienne): Verify with Sofian why whe send a bundle here. There was some commented code in old version here
    //osc::sendBundle();
  }
}

void onAnimation(char* data){
  // Parse location.
  JSONVar animationData = JSON.parse(data);
  JSONVar _baseColor = animationData["base"];
  JSONVar _altColor  = animationData["alt"];


  if (animations::lockMutex()) {
    animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
    animations::currentAnimation().setBaseColor(int(_baseColor[0]), int(_baseColor[1]), int(_baseColor[2]));
    animations::currentAnimation().setAltColor(int(_altColor[0]),  int(_altColor[1]),  int(_altColor[2]));
    animations::currentAnimation().setNoise((float)  double(animationData["noise"]));
    animations::currentAnimation().setPeriod((float) double(animationData["period"]));
    animations::currentAnimation().setType((animations::AnimationType)int(animationData["type"]));
    animations::currentAnimation().setRegion((pixels::Region)int(animationData["region"]) );
    animations::beginTransition();  // start transition
    animations::unlockMutex();
  }
}

}   // namespace mqtt
