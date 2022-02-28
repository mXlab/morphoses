#include <Dynamixel2Arduino.h>
#include <Wire.h>

#define MOTORS_SPEED_MAX 70
#define MOTORS_STEER_MAX 45
#define MOTORS_STEER_MIDDLE 180

// Dynamixel parameters ***********************************
#define DXL_SERIAL   Serial1
const uint8_t DXL_DIR_PIN = 5; // DYNAMIXEL Shield DIR PIN
const uint8_t DXL_ID_SPEED = 1; //motor for rolling
const uint8_t DXL_ID_STEER = 2; // motor for steering left right
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
  dxl.torqueOff(DXL_ID_SPEED);
  dxl.torqueOff(DXL_ID_STEER);
  dxl.setOperatingMode(DXL_ID_SPEED, OP_VELOCITY); // rolling
  dxl.setOperatingMode(DXL_ID_STEER, OP_POSITION); // steering left right
  dxl.torqueOn(DXL_ID_SPEED);
  dxl.torqueOn(DXL_ID_STEER);

  dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_SPEED, profile_acceleration_rolling);
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_SPEED, profile_velocity_rolling);

  dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_STEER, profile_acceleration_steering);
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_STEER, profile_velocity_steering);
}

void setMotorsPower(bool on) {
  if (on) {
    dxl.torqueOn(DXL_ID_SPEED);
    dxl.torqueOn(DXL_ID_STEER);
  } else {
    dxl.torqueOff(DXL_ID_SPEED);
    dxl.torqueOff(DXL_ID_STEER);
  }
}

void processMotors()
{
}

// Remaps normalised value in [-1, 1] to [midPoint-maxRange, midPoint+maxRange].
int safeRemapNorm(float unitVal, int maxRange, int midPoint=0) {
  float remappedVal = midPoint + constrain(unitVal, -1, 1) * maxRange;
  return round(remappedVal);
}

void setMotorsSpeed(float speed) {
  dxl.setGoalVelocity(DXL_ID_SPEED, safeRemapNorm(speed, MOTORS_SPEED_MAX), UNIT_RAW); // +n=CCW, -n=CW
}

void setMotorsSteer(float steer) {
  dxl.setGoalPosition(DXL_ID_STEER, safeRemapNorm(steer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);
}

float getBatteryVoltage() {
  return dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED) / 10.0f;
}

int getMotorSpeedTemperature() {
  return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_SPEED);
}

int getMotorSteerTemperature() {
  return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_STEER);
}

void sendMotorsInfo() {
  bndl.add("/info/battery").add(getBatteryVoltage());
  bndl.add("/info/voltage").add(dxl.readControlTableItem(PRESENT_VOLTAGE, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_VOLTAGE, DXL_ID_STEER));
  bndl.add("/info/temperature").add(getMotorSpeedTemperature()).add(getMotorSteerTemperature());
  bndl.add("/info/current").add(dxl.readControlTableItem(PRESENT_CURRENT, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_CURRENT, DXL_ID_STEER));
  bndl.add("/info/load").add(dxl.readControlTableItem(PRESENT_LOAD, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_LOAD, DXL_ID_STEER));
  sendOscBundle(false, destPortInfo);
}
}
