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

float currentSpeed;
float currentSteer;

bool navigationMode;
float targetHeading;
float targetSpeed;

void initEngine() {
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

  currentSpeed = currentSteer = 0;
  navigationMode = false;
  targetHeading = 0;
  targetSpeed = 0;
}

void setEnginePower(bool on) {
  if (on) {
    dxl.torqueOn(DXL_ID_SPEED);
    dxl.torqueOn(DXL_ID_STEER);
  } else {
    dxl.torqueOff(DXL_ID_SPEED);
    dxl.torqueOff(DXL_ID_STEER);
  }
}
void setEngineSpeed(float speed) {
  dxl.setGoalVelocity(DXL_ID_SPEED, safeRemapNorm(speed, MOTORS_SPEED_MAX), UNIT_RAW); // +n=CCW, -n=CW
  currentSpeed = speed;
}

void setEngineSteer(float steer) {
  dxl.setGoalPosition(DXL_ID_STEER, safeRemapNorm(steer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);
  currentSteer = steer;
}

void startEngineHeading(float speed, float relativeHeading=0) {
  // Get current heading.
  float currentHeading = getHeading();

  // Set target heading.
  targetHeading = wrapAngle180(currentHeading - relativeHeading);

  targetSpeed = speed;
  
  // Start navigation mode.
  navigationMode = true;

  prevPosition.set(currPosition);
  velocityTimer.start();
}

#define STEER_MAX 0.5f
#define HEADING_FRONT_TOLERANCE 30
const int HEADING_FRONT_MAX = 90 + HEADING_FRONT_TOLERANCE;
const float STEER_HEADING_FRONT_MAX = sin(radians(HEADING_FRONT_MAX));

void stepEngineHeading() {
  // Check correction. Positive: too much to the left; negative: too much to the right.
  float relativeHeading = wrapAngle180(targetHeading - getHeading());

  // Compute speed.
  // We use a tolerance in order to force the robot to favor moving forward when it is at almost 90 degrees to avoid situations
  // where it just moves forward and backwards forever. It will move forward  at +- (90 + HEADING_FRONT_TOLERANCE).
  float speed = targetSpeed * (abs(relativeHeading) < HEADING_FRONT_MAX ? +1 : -1);

  // Base steering in [-1, 1] according to relative heading.
  float baseSteer = sin(radians(relativeHeading));

  // Decompose base steer in sign and absolute value.
  float steerSign = copysignf(1, baseSteer);
  float steerValue = abs(baseSteer);

  // Recompute steer in [-1, 1] based on clamped value.
  float steer = steerSign * STEER_MAX * constrain(steerValue/STEER_HEADING_FRONT_MAX, 0, 1);

  setEngineSpeed(speed);
  setEngineSteer(steer);
}

void stopEngineHeading() {
  updateLocation(currentSpeed >= 0);

  setEngineSpeed(0);
  setEngineSteer(0);
  
  navigationMode = false;
  targetHeading = 0;
  targetSpeed = 0;
}

void processEngine()
{
  if (navigationMode) {
    stepEngineHeading();
  }
}


float getEngineSpeed() { return currentSpeed; }
float getEngineSteer() { return currentSteer; }

float getBatteryVoltage() {
  return dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED) / 10.0f;
}

int getEngineSpeedTemperature() {
  return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_SPEED);
}

int getEngineSteerTemperature() {
  return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_STEER);
}

void sendEngineInfo() {
  bndl.add("/battery").add(getBatteryVoltage());
  bndl.add("/speed").add(getEngineSpeed());
  bndl.add("/steer").add(getEngineSteer());

  bndl.add("/velocity-heading").add(getVelocityHeading());
  bndl.add("/velocity").add(getVelocity().x).add(getVelocity().y);
//  bndl.add("/info/battery").add(getBatteryVoltage());
//  bndl.add("/info/voltage").add(dxl.readControlTableItem(PRESENT_VOLTAGE, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_VOLTAGE, DXL_ID_STEER));
//  bndl.add("/info/temperature").add(getMotorSpeedTemperature()).add(getMotorSteerTemperature());
//  bndl.add("/info/current").add(dxl.readControlTableItem(PRESENT_CURRENT, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_CURRENT, DXL_ID_STEER));
//  bndl.add("/info/load").add(dxl.readControlTableItem(PRESENT_LOAD, DXL_ID_SPEED)).add(dxl.readControlTableItem(PRESENT_LOAD, DXL_ID_STEER));
  sendOscBundle();
}