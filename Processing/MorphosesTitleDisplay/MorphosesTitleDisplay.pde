/** //<>//
 * Morpohoses
 *
 * Provides real time visualization of the reward values.
 *
 * (c) 2024 Sofian Audry & Marianne Fournieer
 */

import lord_of_galaxy.timing_utils.*;
import mqtt.*;
import oscP5.*;
import netP5.*;

// MQTT broker.
final String MQTT_BROKER = "192.168.0.200";

// Path/topic variables.
final String MQTT_PATH_ROOT = "morphoses";
final String MQTT_TOPIC_BEGIN = "morphoses/all/info/begin";

// Title screen duration (in seconds).
final float TITLE_DURATION = 8.0;

// MQTT communication.
MQTTClient client;

// Stopwatch to measure time.
Stopwatch watch;

// Title of current behavior.
String behaviorTitle = "";

final int TITLE_SIZE = 48;
final int SUBTITLE_SIZE = 32;

void setup() {
  //fullScreen();
  size(800, 600);
  
  // Initialize MQTT.
  client = new MQTTClient(this);
  client.connect("mqtt://" + MQTT_BROKER);
  
  // Create title timer.
  watch = new Stopwatch(this);

  // Image adjustements.
  smooth();
  noCursor();
}

void draw() {

  // Clear the screen.
  background(0);

  if (watch.second() <= TITLE_DURATION) {

    // Write the title.
    fill(255);
    textSize(TITLE_SIZE);
    textAlign(CENTER, CENTER);
    text(behaviorTitle, width/2, height/2);
  }
}

void clientConnected() {
  println("MQTT connected");

  // Subscribe to topics.
  client.subscribe(MQTT_TOPIC_BEGIN);
}

void connectionLost() {
  println("MQTT connection lost");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));

  // Receive begin with title.

  if (topic.equals(MQTT_TOPIC_BEGIN)) {
    // Get title.
    behaviorTitle = new String(payload);

    // Start timer.
    watch.start();
  }
}
