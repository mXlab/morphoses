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

// Robot name.
final String OSC_PATH_PREFIX = "/robot1";

// OSC input port.
final int OSC_PORT = 8001;

// OSC communication.
OscP5 oscP5;

// MQTT broker.
final String MQTT_BROKER = "192.168.0.200";

// Path/topic variables.
final String MQTT_PATH_ROOT = "morphoses";
String MQTT_PATH_PREFIX;
String MQTT_TOPIC_DATA;
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

float minHeight;
float maxHeight;

final float BORDER_TOP    = ROBOT_LOGO_SIZE + 2*ROBOT_LOGO_OFFSET;
final float BORDER_BOTTOM = 2*ROBOT_LOGO_OFFSET + SUBTITLE_SIZE;

void setup() {
  fullScreen(P2D, 0);

  // Initialize.
  client = new MQTTClient(this);
  //client.connect("mqtt://" + MQTT_BROKER);
  oscP5 = new OscP5(this, OSC_PORT);

  watch = new Stopwatch(this);
  values = new ArrayList<float[]>();
  
  // NEED 3 ARRAYS FOR ALL 3 ROBOTS
  //values1 = new ArrayList<float[]>();
  //values2 = new ArrayList<float[]>();
  //values3 = new ArrayList<float[]>();

  
  robot1 = new Robot("robot1", 0, width, 0, height/3);
  robot2 = new Robot("robot2", 0, width, height/3, 2*height/3);
  robot3 = new Robot("robot3", 0, width, 2*height/3, height);


  // Read robot name.
  String robotName = loadStrings("robot_name.txt")[0];
  // Set topics variables.
  MQTT_PATH_PREFIX = MQTT_PATH_ROOT + "/" + robotName;
  MQTT_TOPIC_DATA  = MQTT_PATH_PREFIX + "/info/display-data";
  MQTT_TOPIC_BEGIN = MQTT_PATH_ROOT + "/all/info/begin";

  // Load corresponding image.
  robotLogo = loadImage(robotName + "_blanc_fond_noir.jpg");

  // Image adjustements.
  smooth();
  noCursor();
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
    
   robot1.drawGraph();
   robot2.drawGraph();
   robot3.drawGraph();

    // Write the title.
    if (watch.second() >= SUBTITLE_START_TIME) {
      textAlign(RIGHT, TOP);
      fill(192);
      textSize(SUBTITLE_SIZE);
      text(behaviorTitle, width - ROBOT_LOGO_OFFSET, height - BORDER_BOTTOM + ROBOT_LOGO_OFFSET);
    }
  }
}

//float yToReward(float y) {
//  if (minReward == maxReward)
//    return minReward;
//  else
//    return map(reward, minHeight, maxHeight, minReward, maxReward);
//}

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
  client.subscribe(MQTT_TOPIC_DATA);
  client.subscribe(MQTT_TOPIC_BEGIN);
}

void connectionLost() {
  println("MQTT connection lost");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
  // Receive data.
  if (topic.equals(MQTT_TOPIC_DATA)) {
    nValuesReceived++;
    if (nValuesReceived >= 2) // drop the first one
      values.add(getValues(parseJSONArray(new String(payload))));

    // Receive begin with titlee.
  } else if (topic.equals(MQTT_TOPIC_BEGIN)) {
    nValuesReceived = 0;
    minReward = +9999;
    maxReward = -9999;

    // Get title.
    behaviorTitle = new String(payload);

    // Reinitialize values.
    values.clear();

    // Switch to title mode and start timer.
    titleMode = true;
    watch.start();
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

// OSC event.
void oscEvent(OscMessage msg) {
  // Receive information point.
  if (msg.checkAddrPattern(OSC_PATH_PREFIX + "/info")) {
    nValuesReceived++;
    if (nValuesReceived > 5)
      values.add(getOSCValues(msg));
  }

  // Begin new behavior.
  else if (msg.checkAddrPattern("/all/begin")) {
    // Get title.
    behaviorTitle = msg.get(0).stringValue();

    // Reinitialize values.
    values.clear();

    // Switch to title mode and start timer.
    titleMode = true;
    watch.start();
  }
}
