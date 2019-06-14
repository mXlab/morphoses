import oscP5.*;
import netP5.*;

final int OSC_SEND_PORT = 8765;
final int OSC_RECV_PORT = 8767;
final String OSC_IP     = "192.168.4.1";

final int MAX_ABS_VALUE       = 1200;
final int VALUE_PER_GRID_LINE = 100;
final int DOT_SIZE            = 30;

OscP5 oscP5;
NetAddress remoteLocation;

ArrayList<PVector> data = new ArrayList<PVector>();

float yaw, pitch, roll;

void setup() {
  size(800, 800);
  lights();
  smooth();
 
  // Start oscP5, listening for incoming messages.
  oscP5 = new OscP5(this, OSC_RECV_PORT);

  // Location to send OSC messages
  remoteLocation = new NetAddress(OSC_IP, OSC_SEND_PORT);
 
  println("Ask to stream");
  OscMessage msg = new OscMessage("/stream");
  oscP5.send(msg, remoteLocation);
  
  oscP5.plug(this, "acceleration", "/accel/g");
  oscP5.plug(this, "gyro",         "/gyro/ds");
  oscP5.plug(this, "magnetism",    "/mag/mG");
  oscP5.plug(this, "yawPitchRoll", "/ypr/deg");
}

void draw() {
  background(0);
  noStroke();
  
  translate(width/2, height/2);
  scale(width/(float)(MAX_ABS_VALUE*2));
  textSize(VALUE_PER_GRID_LINE/4);

  // Draw grid.
  stroke(128);
  fill(128);
  for (int i=-MAX_ABS_VALUE; i<=MAX_ABS_VALUE; i+=VALUE_PER_GRID_LINE) {
    text(i, i, 0);
    line(i, -MAX_ABS_VALUE, i, MAX_ABS_VALUE);
    text(i, 0, i);
    line(-MAX_ABS_VALUE, i, MAX_ABS_VALUE, i);
  }
  
  // Draw all data points.
  ArrayList<PVector> dataCopy = null;
  synchronized(data) {
    dataCopy = new ArrayList<PVector>(data);
  }
  for (PVector v : dataCopy) {
    fill(255, 0, 0, 128);
    ellipse(v.x, v.y, DOT_SIZE, DOT_SIZE);
    fill(0, 255, 0, 128);
    ellipse(v.x, v.z, DOT_SIZE, DOT_SIZE);
    fill(0, 0, 255, 128);
    ellipse(v.y, v.z, DOT_SIZE, DOT_SIZE);
  }
}

void acceleration(float ax, float ay, float az) {
  println("Acceleration:      " + ax + " " + ay + " " + az);
}

void gyro(float gx, float gy, float gz) {
  println("Gyro:              " + gx + " " + gy + " " + gz);
}

void magnetism(float mx, float my, float mz) {
  println("Magnetism:         " + mx + " " + my + " " + mz);
  data.add(new PVector(mx, my, mz));
}

void yawPitchRoll(float yaw, float pitch, float roll) {
  println("Yaw/Pitch/Roll:    " + yaw + " " + pitch + " " + roll);
  this.yaw = yaw;
  this.pitch = pitch;
  this.roll = roll;
}

void keyPressed() {
  if (key == ' ')
    data.clear();
}

void oscEvent(OscMessage msg) {
  /* print the address pattern and the typetag of the received OscMessage */
/*  print("### received an osc message.");
  print(" addrpattern: "+msg.addrPattern());
  println(" typetag: "+msg.typetag());*/
}