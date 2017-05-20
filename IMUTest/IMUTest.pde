import oscP5.*;
import netP5.*;

final int OSC_SEND_PORT = 8765;
final int OSC_RECV_PORT = 8767;
final String OSC_IP     = "192.168.1.109";

OscP5 oscP5;
NetAddress remoteLocation;

RoboBall robot;

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
  
//  oscP5.plug(this, "acceleration", "/accel/g");
//  oscP5.plug(this, "gyro", "/gyro/ds");
//  oscP5.plug(this, "magnetism", "/mag/mG");
  oscP5.plug(this, "yawPitchRoll", "/ypr/deg");
  
  robot = new RoboBall();
}

void draw() {
  processInteractive();
  
  background(0);
  
  translate(width/2, height/2);
  lights();
  
  robot.draw();


}

void processInteractive() {
  if (keyPressed) {
    switch (key) {
      case '4': robot.setYaw(robot.getYaw()-1); break;
      case '6': robot.setYaw(robot.getYaw()+1); break;
      case '8': robot.setPitch(robot.getPitch()-1); break;
      case '2': robot.setPitch(robot.getPitch()+1); break;
      case '7': robot.setRoll(robot.getRoll()-1); break;
      case '9': robot.setRoll(robot.getRoll()+1); break;
    }
  }
}

void keyPressed() {
  if (key == 's')
    robot.syncOffsets();
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
  robot.setYaw(yaw);
  robot.setPitch(pitch);
  robot.setRoll(roll);
}

void oscEvent(OscMessage msg) {
  /* print the address pattern and the typetag of the received OscMessage */
/*  print("### received an osc message.");
  print(" addrpattern: "+msg.addrPattern());
  println(" typetag: "+msg.typetag());*/
}