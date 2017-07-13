import oscP5.*;
import netP5.*;

final int OSC_SEND_PORT = 8765;
final int OSC_RECV_PORT = 8767;
final String OSC_IP     = "192.168.1.109";

OscP5 oscP5;
NetAddress remoteLocation;

RoboBall robot;

void setup() {
  size(720, 480, P3D);
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
  oscP5.plug(this, "magnetism", "/mag/mG");
  oscP5.plug(this, "quaternion", "/quat");
  oscP5.plug(this, "yawPitchRoll", "/ypr/deg");
  
  robot = new RoboBall();
}

void draw() {
  processInteractive();
  
  background(0);
  
  
  robot.draw();


}

void processInteractive() {
  if (keyPressed) {
    switch (key) {
      case '4': robot.setYaw(robot.getYaw()+1); break;
      case '6': robot.setYaw(robot.getYaw()-1); break;
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
  else if (key == 'r')
    robot.reset();
}

void acceleration(float ax, float ay, float az) {
  println("Acceleration:      " + ax + " " + ay + " " + az);
}

void gyro(float gx, float gy, float gz) {
  println("Gyro:              " + gx + " " + gy + " " + gz);
}

void magnetism(float mx, float my, float mz) {
  println("Magnetism:         " + mx + " " + my + " " + mz);
  robot.setMag(mx, my, mz);
}

void quaternion(float q0, float q1, float q2, float q3) {
  // Prevent Gimball lock.
  float test = q0*q2 - q3*q1;
  float yaw, pitch, roll;
  if (test > 0.499f) {
    yaw   =   2 * atan2(q0, q3);
    pitch =   PI/2;
    roll  =   0;
  }
  else if (test < -0.499f) {
    yaw   = - 2 * atan2(q0, q3);
    pitch = - PI/2;
    roll  =   0;
  }
  else {
    yaw   =   atan2( 2 * (q0*q3 + q1*q2), 1 - 2 * (sq(q2) + sq(q3)) );
    pitch =   asin ( 2 * test );
    roll  =   atan2( 2 * (q0*q1 + q2*q3), 1 - 2 * (sq(q1) + sq(q2)) );
  }

  yaw   *= RAD_TO_DEG;
  pitch *= RAD_TO_DEG;
  roll  *= RAD_TO_DEG;

  // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
  //   8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
  // - http://www.ngdc.noaa.gov/geomag-web/#declination
  //myIMU.yaw   -= 8.5;
  // Declination of Concordia University EV Building is
  // 14.47° W  ± 0.38° on 2017-01-31
  // - http://www.ngdc.noaa.gov/geomag-web/#declination
  yaw   -= -14.47;
  
  robot.setYaw(yaw);
  robot.setPitch(pitch);
  robot.setRoll(roll);
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