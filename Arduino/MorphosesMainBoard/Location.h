#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <Arduino_JSON.h>

#include <VectorXf.h>

// Create an ESP32 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_BROKER_PORT);

// Setup a feed called 'location' for subscribing to current location.
Adafruit_MQTT_Subscribe location(&mqtt, "dwm/node/1a1e/uplink/location");

#define POSITION_ALPHA 1.0

const Vec2f REFERENCE_ORIENTATION(1, 0);

Vec2f currPosition;
Vec2f prevPosition;

Vec2f currVelocity;

float velocityHeading;

Chrono velocityTimer;

void onMqttLocation(char* data, uint16_t len)
{
  JSONVar location = JSON.parse(data);
  JSONVar position = location["position"];
  if ((int)position["quality"] > 50) {

    // Set current position.
    Vec2f newPosition((float)double(position["x"]), (float)double(position["y"]));

    // Damping.
    currPosition += POSITION_ALPHA * ( newPosition - currPosition );
    
    bndl.add("/pos").add(currPosition.x).add(currPosition.y);
    sendOscBundle();
  }
}

void initMqtt() {

  // Connect client.
  location.setCallback(onMqttLocation);
  
  // Setup MQTT subscription for time feed.
  mqtt.subscribe(&location);

  currPosition.set(0, 0);
  prevPosition.set(0, 0);
  velocityHeading = 0;
  
  velocityTimer.start();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void mqttConnect() {
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
  mqttConnect();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(10);
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

void updateLocation(boolean movingForward) {
  float t = velocityTimer.elapsed() / 1000.0f;

  if (t > 0) {
//    currVelocity.x = (currPosition.x - prevPosition.x) / t;
//    currVelocity.y = (currPosition.y - prevPosition.y) / t;
    currVelocity = currPosition - prevPosition;
    velocityHeading = REFERENCE_ORIENTATION.angle(currVelocity);
    if (!movingForward) velocityHeading = wrapAngle180(velocityHeading + 180);
  }
  else {
    currVelocity.set(0, 0);
  }

  // Reset.
  bndl.add("/curr-pos").add(currPosition.x).add(currPosition.y);
  bndl.add("/prev-pos").add(prevPosition.x).add(prevPosition.y);
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
