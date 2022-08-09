#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <Arduino_JSON.h>

#include <VectorXf.h>

// Create an ESP32 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_BROKER_PORT);

// Setup a feed called 'location' for subscribing to current location.
Adafruit_MQTT_Subscribe* robotLocations[N_ROBOTS];
//Adafruit_MQTT_Subscribe thingLocations[N_THINGS];

//#define POSITION_ALPHA 1.0

const Vec2f REFERENCE_ORIENTATION(1, 0);

Vec2f robotPositions[N_ROBOTS];

Vec2f currPosition;
Vec2f prevPosition;
Vec2f currVelocity;

float velocityHeading;
Chrono velocityTimer;

Smoother avgPositionX(0.2f);
Smoother avgPositionY(0.2f);
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

    // Damping.
    if (robot+1 == robotId) {
      currPosition.set(newPosition);
//      bndl.add("/pos").add(currPosition.x).add(currPosition.y);
//      currPosition += POSITION_ALPHA * ( newPosition - currPosition );
    }
    
//    bndl.add("/pos").add(robot+1).add(robotPositions[robot].x).add(robotPositions[robot].y);
      
    sendOscBundle();
  }
}

void initMqtt() {
  // Subscrive to RTLS robot location messages.
  for (int i=0; i<N_ROBOTS; i++) {
    // Create subscription.
    sprintf(ROBOT_RTLS_MQTT_ADDRESS[i], "dwm/node/%s/uplink/location", ROBOT_RTLS_IDS[i]);
    robotLocations[i] = new Adafruit_MQTT_Subscribe(&mqtt, ROBOT_RTLS_MQTT_ADDRESS[i]);

    // Subscribe.
    mqtt.subscribe(robotLocations[i]);
  }
  
  currPosition.set(0, 0);
  prevPosition.set(0, 0);
  avgPosition.set(0, 0);
  velocityHeading = 0;

  avgPositionX.reset();
  avgPositionY.reset();
  
  velocityTimer.start();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connectMqtt() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void updateMqtt() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  connectMqtt();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(100)) {
    for (int i=0; i<N_ROBOTS; i++) {
      if (subscription == robotLocations[i]) {
        onMqttLocation(i, (char *)robotLocations[i]->lastread);
        break;
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

void updateVelocity(boolean movingForward) {
  float t = velocityTimer.elapsed();

  if (t > 0) {
    currVelocity = (avgPosition - prevPosition) / t;
    velocityHeading = REFERENCE_ORIENTATION.angle(currVelocity);
    if (!movingForward) velocityHeading = wrapAngle180(velocityHeading + 180);
  }
  else {
    currVelocity.set(0, 0);
  }

  // Reset previous position.
  prevPosition.set( avgPosition );
}

Vec2f getPosition() {
  return currPosition;
}

Vec2f getVelocity() {
  return currVelocity;
}

float getVelocityHeading() {
  return velocityHeading;
}
