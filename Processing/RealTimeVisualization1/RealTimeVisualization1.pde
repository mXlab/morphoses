/**
 * oscP5plug by andreas schlegel
 * example shows how to use the plug service with oscP5.
 * the concept of the plug service is, that you can
 * register methods in your sketch to which incoming 
 * osc messages will be forwareded automatically without 
 * having to parse them in the oscEvent method.
 * that a look at the example below to get an understanding
 * of how plug works.
 * oscP5 website at http://www.sojamo.de/oscP5
 */

import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;

ArrayList<float[]> values1;

float minReward = +9999;
float maxReward = -9999;

void setup() {
//  fullScreen();
  size(1920, 1080);
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this, 8001);

  values1 = new ArrayList<float[]>();
  smooth();
}

int N_POINTS = 100;
void draw() {
  background(0);
  if (values1.size() >= 1) {
  
    float[] prevPoint = getY(values1, values1.size()-1);
    float prevX = width-1;

    for (int i=1; i<N_POINTS; i++) {
      float x = map(i, 1, N_POINTS-1, width-1, 0);
      int k = (values1.size()-1)-i;
      if (0 <= k && k < values1.size()) {
        strokeWeight(1);
        float[] point = getY(values1, k);
        for (int j=0; j<point.length; j++) {
          if (j == point.length-1)
            stroke(0, 255, 0);
          else
            stroke(255, 128);
          
          line(x, point[j], prevX, prevPoint[j]);
        }
        prevPoint = point;
      }
      prevX = x;
    }
  }
}

float[] getY(ArrayList<float[]> array, int i) {
  float[] point = array.get(i);
  float[] ys = new float[point.length];
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
  return ys;
}

float[] getValues(OscMessage msg) {
  int nValues = msg.typetag().length();

  float[] values = new float[nValues];
  for (int i=0; i<nValues; i++) {
    values[i] = msg.get(i).floatValue();
    if (i == nValues - 1) {
      minReward = min(minReward, values[i]);
      maxReward = max(maxReward, values[i]);
    }
  }
  return values;
}

int nReceived = 0;

/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage msg) {
  if (msg.checkAddrPattern("/info/robot1")) {
    nReceived++;
    if (nReceived > 5)
      values1.add(getValues(msg));
  }
}
