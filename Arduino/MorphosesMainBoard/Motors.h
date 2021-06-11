#include <Dynamixel2Arduino.h>

// Dynamixel parameters ***********************************
#define DXL_SERIAL   Serial1
const uint8_t DXL_DIR_PIN = 5; // DYNAMIXEL Shield DIR PIN
const uint8_t DXL_ID = 1; //motor for rolling
const uint8_t DXL_ID2 = 2; // motor for steering left right
const float DXL_PROTOCOL_VERSION = 2.0;

// Dynamixel motor control object.
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

//This namespace is required to use Control table item names
using namespace ControlTableItem;

uint32_t profile_acceleration_rolling = 30;
uint32_t profile_velocity_rolling = 180;

uint32_t profile_acceleration_steering = 40;
uint32_t profile_velocity_steering = 150;
// ********************************************************

void initMotors() {
  // Set Port baudrate to 57600bps for DYNAMIXEL motors.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_ID);
  dxl.torqueOff(DXL_ID2);
  dxl.setOperatingMode(DXL_ID, OP_VELOCITY); // rolling
  dxl.setOperatingMode(DXL_ID2, OP_POSITION); // steering left right
  dxl.torqueOn(DXL_ID);
  dxl.torqueOn(DXL_ID2);

  dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID, profile_acceleration_rolling);
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, profile_velocity_rolling);

  dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID2, profile_acceleration_steering);
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID2, profile_velocity_steering);
}

void setMotorPower(bool on) {
  if (on) {
    dxl.torqueOn(DXL_ID);
    dxl.torqueOn(DXL_ID2);
  } else {
    dxl.torqueOff(DXL_ID);
    dxl.torqueOff(DXL_ID2);
  }
}

#define MAX_SPEED 70
void setMotorSpeed(float speed) {
  int velocity = constrain(round(speed * MAX_SPEED), -MAX_SPEED, MAX_SPEED);
  dxl.setGoalVelocity(DXL_ID, velocity, UNIT_RAW); // +n=CCW, -n=CW
}

#define MAX_STEER 45
void setMotorSteer(float steer) {
  int angle = 180 + constrain(round(steer * MAX_STEER), -MAX_STEER, MAX_STEER);
  dxl.setGoalPosition(DXL_ID2, angle, UNIT_DEGREE);
}
