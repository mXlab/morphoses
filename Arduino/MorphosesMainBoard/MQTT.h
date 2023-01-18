#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <Arduino_JSON.h>

#include <VectorXf.h>

#define AVG_POSITION_TIME_WINDOW 0.2f

// Create an ESP32 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_BROKER_PORT);

// Setup a feed called 'location' for subscribing to current location.
Adafruit_MQTT_Subscribe* mqttRobotLocations[N_ROBOTS];
Adafruit_MQTT_Subscribe* mqttAnimationData;


Vec2f robotPositions[N_ROBOTS];
Vec2f currPosition;

Smoother avgPositionX(AVG_POSITION_TIME_WINDOW);
Smoother avgPositionY(AVG_POSITION_TIME_WINDOW);
Vec2f avgPosition;

void onMqttLocation(int robot, char* data)
{
  // Parse location.
  JSONVar location = JSON.parse(data);
  JSONVar position = location["position"];
  if ((int)position["quality"] > 50) {

    // Set current position.
    Vec2f newPosition((float)double(position["x"]), (float)double(position["y"]));

    robotPositions[robot].set(newPosition);

    // Assign.
    if (robot+1 == robotId) {

      currPosition.set(newPosition);
//      bndl.add("/pos").add(currPosition.x).add(currPosition.y);
//      currPosition += POSITION_ALPHA * ( newPosition - currPosition );
    }
    
//    bndl.add("/pos").add(robot+1).add(robotPositions[robot].x).add(robotPositions[robot].y);
      
    sendOscBundle();
  }
}

void onMqttAnimation(char* data) {
  // Parse location.
  JSONVar animationData = JSON.parse(data);
  JSONVar baseColor = animationData["base"];
  JSONVar altColor  = animationData["alt"];
  
  if (lockAnimationMutex()) {
    animation.setBaseColor(int(baseColor[0]), int(baseColor[1]), int(baseColor[2]));
    animation.setAltColor (int(altColor[0]),  int(altColor[1]),  int(altColor[2]));
    animation.setNoise(( float)  double(animationData["noise"]));
    animation.setPeriod((float) double(animationData["period"]));
    animation.setType( (AnimationType)int(animationData["type"]) );
    animation.setRegion( (PixelRegion)int(animationData["region"]) );
    
    unlockAnimationMutex();
  }
}

void initMqtt() {
  // Subscribe to RTLS robot location messages.
  for (int i=0; i<N_ROBOTS; i++) {
    // Create subscription.
    sprintf(ROBOT_RTLS_MQTT_ADDRESS[i], "dwm/node/%s/uplink/location", ROBOT_RTLS_IDS[i]);
    mqttRobotLocations[i] = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_RTLS_MQTT_ADDRESS[i]);

    // Subscribe.
    mqtt.subscribe(mqttRobotLocations[i]);
  }

  // Subscribe to own messages.
  sprintf(ROBOT_CUSTOM_MQTT_ADDRESS, "morphoses/%s/animation", boardName);
  mqttAnimationData = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_CUSTOM_MQTT_ADDRESS);
  mqtt.subscribe(mqttAnimationData);
  
  currPosition.set(0, 0);

  avgPosition.set(0, 0);
  avgPositionX.reset();
  avgPositionY.reset();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connectMqtt() {

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }


  Serial.print("Connecting to MQTT... ");
  int8_t error = mqtt.connect();
  if (error) { // error detected
    Serial.println(mqtt.connectErrorString(error));

 #define MQTT_ERROR_STRING_SIZE 64
    char errorStr[MQTT_ERROR_STRING_SIZE];
    strncpy_P(errorStr, (PGM_P)mqtt.connectErrorString(error), MQTT_ERROR_STRING_SIZE);

    // Send error
    bndl.add("/error").add("mqtt-connect").add(errorStr);
    sendOscBundle();

    mqtt.disconnect();
  }

  else {
    bndl.add("/ready").add("mqtt-connect");
    sendOscBundle();
    Serial.println("MQTT Connected!");
  }
}

void updateMqtt() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  connectMqtt();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(10)) {

    if (subscription == mqttAnimationData) {
      onMqttAnimation((char *)mqttAnimationData->lastread);
    }
    else {
      for (int i=0; i<N_ROBOTS; i++) {
        if (subscription == mqttRobotLocations[i]) {
          onMqttLocation(i, (char *)mqttRobotLocations[i]->lastread);
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

void updateLocation() {
  // Update average positioning.
  avgPositionX.put( currPosition.x );
  avgPositionY.put( currPosition.y );
  avgPosition.set( avgPositionX.get(), avgPositionY.get() );
}


Vec2f getPosition() {
  return avgPosition;
}
