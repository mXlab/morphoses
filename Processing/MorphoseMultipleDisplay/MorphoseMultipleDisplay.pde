/** //<>//
 * Morpohoses
 *
 * Provides real time visualization of the reward values.
 *
 * (c) 2024 Sofian Audry
 */

import lord_of_galaxy.timing_utils.*;
import mqtt.*;
import oscP5.*;
import netP5.*;

// Robots
Robot robot1;
Robot robot2;
Robot robot3;

// OSC input port.
final int OSC_PORT = 8001;

// OSC communication.
OscP5 oscP5;

// MQTT broker.
final String MQTT_BROKER = "192.168.0.200";

// Path/topic variables.
final String MQTT_PATH_ROOT = "morphoses";
final String MQTT_PATH_SUFFIX = "/info/display-data";
String[] MQTT_TOPIC_DATA;
String MQTT_TOPIC_BEGIN;

// Number of points to be plotted.
final int N_POINTS = 100;

// Title screen duration (in seconds).
final float TITLE_DURATION = 8.0;
final float SUBTITLE_START_TIME = TITLE_DURATION + 4.0;

// MQTT communication.
MQTTClient client;

// Number of values received.
int nValuesReceived;

// Contains all the values to be plotted.
ArrayList<float[]> values;

// Min. and max. reward values.
float minReward = +9999;
float maxReward = -9999;

// Stopwatch to measure time.
Stopwatch watch;

// Mode: title vs graph.
boolean titleMode = false;

// Title of current behavior.
String behaviorTitle = "";

PImage robotLogo;
final int ROBOT_LOGO_SIZE = 100;
final int ROBOT_LOGO_OFFSET = 50;
final float ROBOT_LOGO_SHAKE = 1;

final int TITLE_SIZE = 48;
final int SUBTITLE_SIZE = 32;

final color COLOR_REWARD_MAX = #00ff00;
final color COLOR_REWARD_MIN = #ff0000;
final float LINE_WEIGHT_REWARD = 3;

final color COLOR_DATA = color(255, 48);
final float LINE_WEIGHT_DATA = 3;

float speed;
float graphPos;

float minHeight;
float maxHeight;

final float BORDER_TOP    = ROBOT_LOGO_SIZE + 2*ROBOT_LOGO_OFFSET;
final float BORDER_BOTTOM = 2*ROBOT_LOGO_OFFSET + SUBTITLE_SIZE;

void setup() {
  fullScreen(P2D, 0);

  // Initialize.
  graphPos = 0;
  client = new MQTTClient(this);
  //  client.connect("mqtt://" + MQTT_BROKER);
  oscP5 = new OscP5(this, OSC_PORT);

  watch = new Stopwatch(this);

  robot1 = new Robot(1, 0, width, 0, (height-SUBTITLE_SIZE*3)/3);
  robot2 = new Robot(2, 0, width, (height-SUBTITLE_SIZE*3)/3, 2*(height-SUBTITLE_SIZE*3)/3);
  robot3 = new Robot(3, 0, width, 2*(height-SUBTITLE_SIZE*3)/3, (height-SUBTITLE_SIZE*3));

  // Set topics variables.
  MQTT_TOPIC_DATA  = new String[3];
  MQTT_TOPIC_DATA[0] = MQTT_PATH_ROOT + "/" + robot1.getName() + MQTT_PATH_SUFFIX; //robot1
  MQTT_TOPIC_DATA[1] = MQTT_PATH_ROOT + "/" + robot2.getName() + MQTT_PATH_SUFFIX; //robot2
  MQTT_TOPIC_DATA[2] = MQTT_PATH_ROOT + "/" + robot3.getName() + MQTT_PATH_SUFFIX; //robot3
  MQTT_TOPIC_BEGIN = MQTT_PATH_ROOT + "/all/info/begin";

  // Image adjustements.
  smooth();
  noCursor();
  speed = 3;
}

void draw() {
  // Clear the screen.
  background(0);

  ///// TITLE MODE //////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////

  if (titleMode) {
    // Write the title.
    fill(255);
    textSize(TITLE_SIZE);
    textAlign(CENTER, CENTER);
    text(behaviorTitle, width/2, height/2);

    // Check if the title duration has passed.
    if (watch.second() >= TITLE_DURATION)
      titleMode = false; // Switch to graph mode.
  }

  //// GRAPH MODE //////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  else {

    //// DRAW LOGO /////
    ////////////////////

    robot1.drawLogo();
    robot2.drawLogo();
    robot3.drawLogo();

    ////// DRAW GRAPH ////
    //////////////////////
    pushMatrix();
    //translate(graphPos,0);
    robot1.drawGraphTest();
    robot2.drawGraph();
    robot3.drawGraph();
    popMatrix();
    
    graphPos-=speed;

    // Write the title.
    writeTitle();
  }
}

//float yToReward(float y) {
//  if (minReward == maxReward)
//    return minReward;
//  else
//    return map(reward, minHeight, maxHeight, minReward, maxReward);
//}

void writeTitle() {
  if (watch.second() >= SUBTITLE_START_TIME) {
    textAlign(RIGHT, TOP);
    fill(192);
    textSize(SUBTITLE_SIZE);
    text(behaviorTitle, width - ROBOT_LOGO_OFFSET, height - BORDER_BOTTOM + ROBOT_LOGO_OFFSET);
  }
}

color getColorReward(float reward) {
  return lerpColor(COLOR_REWARD_MIN, COLOR_REWARD_MAX, reward);
}

// Extract floating point values from message and return them as an array.
float[] getValues(JSONArray array) {
  // Get n. values.
  int nValues = array.size();

  // Extract values.
  float[] values = new float[nValues];
  for (int i=0; i<nValues; i++) {
    values[i] = array.getFloat(i);

    // Compute min/max reward values.
    if (i == nValues - 1) {
      minReward = min(minReward, values[i]);
      maxReward = max(maxReward, values[i]);
    }
  }

  // Return values.
  return values;
}

void clientConnected() {
  println("MQTT connected");

  // Subscribe to topics.
  client.subscribe(MQTT_TOPIC_DATA[0]);
  client.subscribe(MQTT_TOPIC_DATA[1]);
  client.subscribe(MQTT_TOPIC_DATA[2]);
  client.subscribe(MQTT_TOPIC_BEGIN);
}

void connectionLost() {
  println("MQTT connection lost");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
  // Receive data.
  //if (topic.equals(MQTT_TOPIC_DATA)) {
  //  nValuesReceived++;
  //  if (nValuesReceived >= 2) // drop the first one
  //    values.add(getValues(parseJSONArray(new String(payload))));



  // Receive begin with title.

  if (topic.equals(MQTT_TOPIC_BEGIN)) {
    robot1.reinitializeValues();
    robot2.reinitializeValues();
    robot3.reinitializeValues();
    // Get title.
    behaviorTitle = new String(payload);

    // Switch to title mode and start timer.
    titleMode = true;
    watch.start();

    //receive values
  } else {
    robot1.addValues(topic, payload);
    robot2.addValues(topic, payload);
    robot3.addValues(topic, payload);
  }
}

// Extract OSC vals

float[] getOSCValues(OscMessage msg) {
  // Get n. values.
  int nValues = msg.typetag().length();

  // Extract values.
  float[] OscValues = new float[nValues];
  for (int i=0; i<nValues; i++) {
    OscValues[i] = msg.get(i).floatValue();

    // Compute min/max reward values.
    if (i == nValues - 1) {
      minReward = min(minReward, OscValues[i]);
      maxReward = max(maxReward, OscValues[i]);
    }
  }

  // Return values.
  return OscValues;
}

//OSC event.
void oscEvent(OscMessage msg) {

  // Begin new behavior.
  if (msg.checkAddrPattern("/all/begin")) {
    // Get title.
    behaviorTitle = msg.get(0).stringValue();

    // Reinitialize values.
    values.clear();

    // Switch to title mode and start timer.
    titleMode = true;
    watch.start();
  } else {
    robot1.addValues(msg);
    robot2.addValues(msg);
    robot3.addValues(msg);
  }
}
