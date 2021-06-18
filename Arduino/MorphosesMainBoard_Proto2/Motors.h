#include <Wire.h>

#define MOTORS_SPEED_MAX 255
#define MOTORS_STEER_MAX 45
#define MOTORS_STEER_MIDDLE 0

// Pin definitions for ESP8266Thing

const byte MOTOR1_I2C_ADDRESS = 8; // i2c address of motor 1
const byte MOTOR2_I2C_ADDRESS = 16; // i2c address of motor 2
const byte MOTOR_SPEED = 1; // selector for motor speed
const byte MOTOR_POSITION = 2; // selector for motor encoder position
const byte MOTOR_RESET = 3; // selector for motor home

#define MOTOR_POWER 0

void initMotors() {
  pinMode(MOTOR_POWER, OUTPUT);
  digitalWrite(MOTOR_POWER, HIGH);
}

void setMotorsPower(bool on) {
  digitalWrite(MOTOR_POWER, on ? LOW : HIGH);
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
  int val = safeRemapNorm(speed, MOTORS_SPEED_MAX);
  Wire.beginTransmission(MOTOR1_I2C_ADDRESS); // transmit to device #8
  Wire.write(MOTOR_SPEED); // sends one byte
  Wire.write(val>>24); // send 4 bytes bigendian 32-bit int
  Wire.write(val>>16);
  Wire.write(val>>8);
  Wire.write(val);
  Wire.endTransmission(); // stop transmitting
}

void setMotorsSteer(float steer) {
  int val = safeRemapNorm(steer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE);
  Wire.beginTransmission(MOTOR1_I2C_ADDRESS); // transmit to device #8
  Wire.write(MOTOR_SPEED); // sends one byte
  Wire.write(val>>24); // send 4 bytes bigendian 32-bit int
  Wire.write(val>>16);
  Wire.write(val>>8);
  Wire.write(val);
  Wire.endTransmission(); // stop transmitting
}
