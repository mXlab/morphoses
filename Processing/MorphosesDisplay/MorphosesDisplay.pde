/**
 * Morpohoses
 * 
 * Provides real time visualization of the reward values.
 *
 * (c) 2024 Sofian Audry
 */

import lord_of_galaxy.timing_utils.*;
import mqtt.*;

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
final int robotLogoSize = 100;
final int robotLogoOffset = 50;
final float robotLogoShake = 1;

final color COLOR_REWARD_MAX = #00ff00;
final color COLOR_REWARD_MIN = #ff0000;
final color COLOR_DATA = color(255, 128);

float minHeight;
float maxHeight;

float HEIGHT_MIN;
float HEIGHT_MAX;

void setup() {
  fullScreen(P2D, 0);
//  size(640, 480);
//  size(1920, 1080);
  minHeight = height*0.9;
  maxHeight = height*0.1;

  // Ininialize.
  client = new MQTTClient(this);
  client.connect("mqtt://" + MQTT_BROKER);


  watch = new Stopwatch(this);
  values = new ArrayList<float[]>();

  // Read robot name.
  String robotName = loadStrings("robot_name.txt")[0];
   
  // Set topics variables.
  MQTT_PATH_PREFIX = MQTT_PATH_ROOT + "/" + robotName;
  MQTT_TOPIC_DATA  = MQTT_PATH_PREFIX + "/info/display-data";
  MQTT_TOPIC_BEGIN = MQTT_PATH_ROOT + "/all/info/begin";
  
  // Load corresponding image.
  robotLogo = loadImage(robotName + "_blanc_fond_transparent.png");

  // Smooth drawing.
  smooth();

  // Initialize text parameters.
  textSize(48);
  textAlign(CENTER, CENTER);
}

void draw() {
  // Clear the screen.
  background(0);
  
  // Title mode //////////////////////////////////////////////////////////////
  if (titleMode) {
    // Write the title.
    text(behaviorTitle, width/2, height/2);
    
    // Check if the title duration has passed.
    if (watch.second() >= TITLE_DURATION)
      titleMode = false; // Switch to graph mode.
  }

  // Graph mode //////////////////////////////////////////////////////////////
  else {
    // Draw the graph.
    if (values.size() >= 1) {
    
      // Get the last point added.
      float[] prevPoint = getY(values, values.size()-1);
      float prevX = width-1;

      // Draw the graph from right to left.
      for (int i=1; i<N_POINTS; i++) {
        // Position of i-th point.
        float x = map(i, 1, N_POINTS-1, width-1, 0); // Inverted: right to left.

        // Get the point to the left.
        int k = (values.size()-1) - i;
        if (0 <= k && k < values.size()) {
          // Draw the line segments for each part of the point.
          strokeWeight(1);
          float[] point = getY(values, k);
          for (int j=0; j<point.length; j++) {
            // Determine color.
            if (j == point.length-1) {
              gradientLine(x, point[j], prevX, prevPoint[j]);
            }
            else {
              stroke(COLOR_DATA);
              line(x, point[j], prevX, prevPoint[j]);
            }
          }
          // Update previous point.
          prevPoint = point;
        }
        // Update previous x.
        prevX = x;
      }
    }
    
    // Draw logo.
    if (values.size() > 0) {
      float[] lastPoint = values.get(values.size()-1);
      tint(getColorReward(lastPoint[lastPoint.length-1]));
    }
    else
      tint(getColorReward(0.5));
    image(robotLogo, width-robotLogoSize-robotLogoOffset+random(-robotLogoShake,robotLogoShake), 
                     robotLogoOffset+random(-robotLogoShake,robotLogoShake), 
                     robotLogoSize, robotLogoSize);
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

// Returns the y coordinates of the i-th data element.
float[] getY(ArrayList<float[]> array, int i) {
  // Get data.
  float[] point = array.get(i);
  float[] ys = new float[point.length];

  // Map the data to the screen.
  for (int j=0; j<point.length; j++) {
    if (j == point.length - 1) {
      if (minReward == maxReward)
        ys[j] = map(0.5, 0, 1, minHeight, maxHeight);
      else
        ys[j] = map(point[j], minReward, maxReward, minHeight, maxHeight);
    }
    else
      ys[j] = map(point[j], 0, 1, minHeight, maxHeight);
  }
  // Return the y coordinates.
  return ys;
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

void gradientLine(float x1, float y1, float x2, float y2) {
  // Calculate the number of steps based on the distance between the points
  int steps = int(dist(x1, y1, x2, y2));
  
  for (int i = 0; i <= steps; i++) {
    float t = map(i, 0, steps, 0, 1);
    float x = lerp(x1, x2, t);
    float y = lerp(y1, y2, t);
    
    // Map y position to a value between 0 and 1
    float gradient = map(y, minHeight, maxHeight, 0, 1);
    
    // Calculate the color for the current step
    int currentColor = lerpColor(COLOR_REWARD_MIN, COLOR_REWARD_MAX, gradient);
    
    // Set the stroke color
    stroke(currentColor);
    
    // Draw the point
    point(x, y);
  }
}

void clientConnected() {
  println("MQTT connected"); //<>//
  
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
    if (nValuesReceived > 5)
      values.add(getValues(parseJSONArray(new String(payload))));
      
   // Receive begin with title.
  } else if (topic.equals(MQTT_TOPIC_BEGIN)) {
        // Get title.
    behaviorTitle = new String(payload);
    
    // Reinitialize values.
    values.clear();
    
    // Switch to title mode and start timer.
    titleMode = true;
    watch.start();
  }
}
