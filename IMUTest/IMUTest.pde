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

  float c1 = cos(radians(roll));
  float s1 = sin(radians(roll));
  float c2 = cos(radians(pitch));
  float s2 = sin(radians(pitch));
  float c3 = cos(radians(yaw));
  float s3 = sin(radians(yaw));
  applyMatrix( c2*c3, s1*s3+c1*c3*s2, c3*s1*s2-c1*s3, 0,
               -s2, c1*c2, c2*s1, 0,
               c2*s3, c1*s2*s3-c3*s1, c1*c3+s1*s2*s3, 0,
               0, 0, 0, 1);

  drawArduino();
  //fill(#F179FF);
  //stroke(0);
  //box(200, 50, 100);
  popMatrix();
}

// Source: https://www.arduino.cc/en/Tutorial/Genuino101CurieIMUOrientationVisualiser
void drawArduino()
{
  /* function contains shape(s) that are rotated with the IMU */
  stroke(0, 90, 90); // set outline colour to darker teal
  fill(0, 130, 130); // set fill colour to lighter teal
  box(300, 10, 200); // draw Arduino board base shape

  stroke(0); // set outline colour to black
  fill(80); // set fill colour to dark grey

  translate(60, -10, 90); // set position to edge of Arduino box
  box(170, 20, 10); // draw pin header as box

  translate(-20, 0, -180); // set position to other edge of Arduino box
  box(210, 20, 10); // draw other pin header as box
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