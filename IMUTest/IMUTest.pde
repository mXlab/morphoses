import oscP5.*;
import netP5.*;

final int OSC_SEND_PORT = 8765;
final int OSC_RECV_PORT = 8766;
final String OSC_IP     = "192.168.4.1";

OscP5 oscP5;
NetAddress remoteLocation;

void setup() {
  size(640, 480, P3D);
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
  oscP5.plug(this, "gyro", "/gyro/ds");
  oscP5.plug(this, "magnetism", "/mag/mG");
  oscP5.plug(this, "yawPitchRoll", "/ypr/deg");
}

float yaw, pitch, roll;

void draw() {
  background(0);
  noStroke();
  translate(width/2, height/2);
  pushMatrix();
  fill(#F179FF);
  rotateX(radians(yaw));
  rotateY(radians(pitch));
  rotateZ(radians(roll));
  box(50, 20, 300);
  popMatrix();
}

void acceleration(float ax, float ay, float az) {
  println("Acceleration:      " + ax + " " + ay + " " + az);
}

void gyro(float gx, float gy, float gz) {
  println("Gyro:              " + gx + " " + gy + " " + gz);
}

void magnetism(float mx, float my, float mz) {
  println("Magnetism:         " + mx + " " + my + " " + mz);
}

void yawPitchRoll(float yaw, float pitch, float roll) {
  println("Yaw/Pitch/Roll:    " + yaw + " " + pitch + " " + roll);
  this.yaw = yaw;
  this.pitch = pitch;
  this.roll = roll;
}

void oscEvent(OscMessage msg) {
  /* print the address pattern and the typetag of the received OscMessage */
/*  print("### received an osc message.");
  print(" addrpattern: "+msg.addrPattern());
  println(" typetag: "+msg.typetag());*/
}