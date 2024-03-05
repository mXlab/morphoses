/**
 * Morpohoses
 * 
 * Provides real time visualization of the reward values.
 *
 * (c) 2024 Sofian Audry
 */

import lord_of_galaxy.timing_utils.*;

import oscP5.*;
import netP5.*;

// Robot name.
String OSC_PATH_PREFIX;

// OSC input port.
final int OSC_PORT = 8001;

// Number of points to be plotted.
final int N_POINTS = 100;

// Title screen duration (in seconds).
final float TITLE_DURATION = 8.0;

// OSC communication.
OscP5 oscP5;

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

void setup() {
  fullScreen();
//  size(1920, 1080);

  // Ininialize.
  oscP5 = new OscP5(this, OSC_PORT);  
  watch = new Stopwatch(this);
  values = new ArrayList<float[]>();

  // Read robot name.
  String robotName = loadStrings("robot_name.txt")[0];
  OSC_PATH_PREFIX = "/" + robotName;
  
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
            if (j == point.length-1)
              stroke(0, 255, 0); // Reward.
            else
              stroke(255, 128); // Data.
            
            // Draw segment.
            line(x, point[j], prevX, prevPoint[j]);
          }
          // Update previous point.
          prevPoint = point;
        }
        // Update previous x.
        prevX = x;
      }
    }
  }
}

// Returns the y coordinates of the i-th data element.
float[] getY(ArrayList<float[]> array, int i) {
  // Get data.
  float[] point = array.get(i);
  float[] ys = new float[point.length];

  // Map the data to the screen.
  float minHeight = height*0.9;
  float maxHeight = height*0.1;
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
float[] getValues(OscMessage msg) {
  // Get n. values.
  int nValues = msg.typetag().length();

  // Extract values.
  float[] values = new float[nValues];
  for (int i=0; i<nValues; i++) {
    values[i] = msg.get(i).floatValue();

    // Compute min/max reward values.
    if (i == nValues - 1) {
      minReward = min(minReward, values[i]);
      maxReward = max(maxReward, values[i]);
    }
  }

  // Return values.
  return values;
}

// OSC event.
void oscEvent(OscMessage msg) {
  
  // Receive information point.
  if (msg.checkAddrPattern(OSC_PATH_PREFIX + "/info")) {
    nValuesReceived++;
    if (nValuesReceived > 5)
      values.add(getValues(msg));
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
