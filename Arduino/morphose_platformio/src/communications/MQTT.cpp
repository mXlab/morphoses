#include "MQTT.h"
#include "Morphose.h"
#include <ArduinoLog.h>
#include "osc.h"
#include "lights/Animation.h"
#include "lights/Pixels.h"

namespace mqtt {

const char* broker{"192.168.0.200"};
const int brokerPort {1883};

// Create an ESP32 WiFiClient class to connect to the MQTT server.
WiFiClient client;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, broker, brokerPort);

// Setup a feed called 'location' for subscribing to current location.
Adafruit_MQTT_Subscribe* mqttRobotLocations[N_ROBOTS];
Adafruit_MQTT_Subscribe* mqttAnimationData;

Vec2f robotPositions[N_ROBOTS];
Vec2f newPosition;

int baseColor[3];
int altColor[3];

void initialize() {

  // Subscribe to RTLS robot location messages.
  for (int i=0; i<N_ROBOTS; i++) {
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
  

 //TODO:MOVE TO MORPHOSE
//   currPosition.set(0, 0);
//   avgPosition.set(0, 0);
//   avgPositionX.reset();
//   avgPositionY.reset();
}


void connect() {

  Log.infoln("Connecting to MQTT... ");
  int8_t error = mqtt.connect();
  if (error) { // error detected
    
    char errorStr[64];
    strncpy_P(errorStr, (PGM_P)mqtt.connectErrorString(error), 64);
    Log.errorln(errorStr);


    // Send error
    
    osc::bundle.add("/error").add("mqtt-connect").add(errorStr);
    osc::sendBundle();
        //sendOscBundle();

    mqtt.disconnect();
  }

  else {
    // bndl.add("/ready").add("mqtt-connect");
    // sendOscBundle();
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
     // onMqttAnimation((char *)mqttAnimationData->lastread);
    }
    else 
    {
      for (int i=0; i<N_ROBOTS; i++) {

        if (subscription == mqttRobotLocations[i]) {

            Log.infoln("Location %d updated", i+1);
            if(onMqttLocation(i, (char *)mqttRobotLocations[i]->lastread) && i+1 == morphose::id){
                morphose::setCurrentPosition(newPosition);
            }

            //buffer all robot positions
            robotPositions[i].set(newPosition);
            // TODO: ADD chrono to send robot position priodically or remove.

          break;
        }
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

bool onMqttLocation(int robot, char* data)
{

  // Parse location.
  JSONVar location = JSON.parse(data);
  JSONVar position = location["position"];
  if ((int)position["quality"] > 50) {

    // Set current position.
    newPosition = Vec2f((float)double(position["x"]), (float)double(position["y"])); // weird way to do this
    return true;
  }
  return false;
}

void onMqttAnimation(char* data)
{
  // Parse location.
  JSONVar animationData = JSON.parse(data);
  JSONVar _baseColor = animationData["base"];
  JSONVar _altColor  = animationData["alt"];
  

//TODO : Works for now. Find a way to wrap all in function and pass data without to much redundency. Maybe pass pointer to json data
  if (animations::lockMutex()) {
    animations::previousAnimation().copyFrom(animations::currentAnimation()); // save animation
    
    animations::currentAnimation().setBaseColor(int(baseColor[0]), int(baseColor[1]), int(baseColor[2]));
    animations::currentAnimation().setAltColor (int(altColor[0]),  int(altColor[1]),  int(altColor[2]));
    animations::currentAnimation().setNoise(( float)  double(animationData["noise"]));
    animations::currentAnimation().setPeriod((float) double(animationData["period"]));
    animations::currentAnimation().setType( (animations::AnimationType)int(animationData["type"]) );
    animations::currentAnimation().setRegion( (pixels::Region)int(animationData["region"]) );

  //TODO: Decide if keep in code
//    transitionTimer.duration(double(animationData["transition"]));

    animations::beginTransition(); // start transition
    animations::unlockMutex();
  }

}





}//namespace mqtt 