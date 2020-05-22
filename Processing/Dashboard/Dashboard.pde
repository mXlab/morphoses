import controlP5.*;

import oscP5.*;
import netP5.*;

// Main board (motor control).
final int MAIN_OSC_SEND_PORT = 8765;
final int MAIN_OSC_RECV_PORT = 8766;

// IMU board.
final int POSITION_OSC_SEND_PORT = 8765;
final int POSITION_OSC_RECV_PORT = 8767;

//final String MAIN_OSC_IP     = "192.168.43.28";
final String MAIN_OSC_IP     = "172.20.10.9";
//final String POSITION_OSC_IP = "192.168.1.108";
final String POSITION_OSC_IP     = "172.20.10.8";

final int ROLLER_MAX = 255;
final int ROLLER_MIN = -255;
final boolean invertTilter = true;
final int TILTER_MIN = -30;

OscP5 mainOscP5;
OscP5 positionOscP5;

NetAddress mainLocation;
NetAddress positionLocation;

DataLogger logger;
String outputFileName = "data.csv";

RoboBall robot;

void setup() {
  size(1440, 960, P3D);
  lights();
  smooth();
 
  // Start mainOscP5, listening for incoming messages.
  mainOscP5 = new OscP5(this, MAIN_OSC_RECV_PORT);
  positionOscP5 = new OscP5(this, POSITION_OSC_RECV_PORT);

  // Location to send OSC messages
  mainLocation = new NetAddress(MAIN_OSC_IP, MAIN_OSC_SEND_PORT);
  positionLocation = new NetAddress(POSITION_OSC_IP, POSITION_OSC_SEND_PORT);
 
  println("Ask to stream");
  OscMessage msg = new OscMessage("/stream");
  mainOscP5.send(msg, mainLocation);
  positionOscP5.send(msg, positionLocation);

  mainOscP5.plug(this, "rollerMotorTicks", "/motor/1/ticks");
  mainOscP5.plug(this, "tilterMotorTicks", "/motor/2/ticks");

//  mainOscP5.plug(this, "acceleration", "/accel/g");
//  mainOscP5.plug(this, "gyro", "/gyro/ds");
//  positionOscP5.plug(this, "magnetism", "/mag/mG");
  positionOscP5.plug(this, "quaternion", "/quat");
//  positionOscP5.plug(this, "yawPitchRoll", "/ypr/deg");
  
  robot = new RoboBall();
  
  logger = new DataLogger();
  
  createGui();
}

void draw() {
//  processInteractive();
//  background(0);
//  robot.draw(width/2, height/2, height);
}

void processInteractive() {
  if (keyPressed) {
    switch (key) {
      //case '8': robot.setRollerMotorSpeed(robot.getRollerMotorSpeed()+255); break;
      //case '2': robot.setRollerMotorSpeed(robot.getRollerMotorSpeed()-255); break;
      //case '5': robot.setRollerMotorSpeed(0); break;
      //case '6': robot.setTilterMotorPosition(robot.getTilterMotorPosition()+40); break;
      //case '4': robot.setTilterMotorPosition(robot.getTilterMotorPosition()-40); break;
      //case '4': robot.setYaw(robot.getYaw()+1); break;
      //case '6': robot.setYaw(robot.getYaw()-1); break;
      //case '8': robot.setPitch(robot.getPitch()-1); break;
      //case '2': robot.setPitch(robot.getPitch()+1); break;
      //case '7': robot.setRoll(robot.getRoll()-1); break;
      //case '9': robot.setRoll(robot.getRoll()+1); break;
      default:;
    }
  }
}


void keyPressed() {
  switch (key) {
    case 's': robot.syncOffsets(); break;
    case 'r': robot.reset(); break;
    case '8': robot.setRollerMotorSpeed(255); break;
    case '2': robot.setRollerMotorSpeed(-255); break;
    case '5': robot.setRollerMotorSpeed(0); robot.setTilterMotorPosition(0); break;
    case '6': robot.setTilterMotorPosition(40); break;
    case '4': robot.setTilterMotorPosition(-40); break;
    case 'q': {
      saveOutput();
      exit(); // Stops the program
    }
    break;
    default:;
  }
}

void recordState() {
  logger.recordState();
}

void saveOutput() {
  logger.save(outputFileName);
}

//void acceleration(float ax, float ay, float az) {
//  println("Acceleration:      " + ax + " " + ay + " " + az);
//  recordState();
//}

//void gyro(float gx, float gy, float gz) {
//  println("Gyro:              " + gx + " " + gy + " " + gz);
//  recordState();
//}

void magnetism(float mx, float my, float mz) {
  println("Magnetism:         " + mx + " " + my + " " + mz);
  robot.setMag(mx, my, mz);
  recordState();
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
  
  recordState();
}

void yawPitchRoll(float yaw, float pitch, float roll) {
  println("Yaw/Pitch/Roll:    " + yaw + " " + pitch + " " + roll);
  robot.setYaw(yaw);
  robot.setPitch(pitch);
  robot.setRoll(roll);

  recordState();
}

void rollerMotorTicks(int t) {
  robot.setRollerMotorTicks(t); 
  recordState();
}

void tilterMotorTicks(int t) {
  robot.setTilterMotorTicks(t); 
  recordState();
}

void oscEvent(OscMessage msg) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+msg.addrPattern());
  println(" typetag: "+msg.typetag());
}