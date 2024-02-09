#ifndef MORPHOSE_ENGINE_H
#define MORPHOSE_ENGINE_H


#include <Dynamixel2Arduino.h>
#include <Wire.h>

#include "Utils.h"

namespace motors{

  void initEngine();
  void setEnginePower(bool on);
  void setEngineSpeed(float speed);
  void setEngineSteer(float steer);
  float getEngineSpeed();
  float getEngineSteer();
  float engineIsMovingForward();
  float getBatteryVoltage();
  int getEngineSpeedTemperature();
  int getEngineSteerTemperature();
  void processEngine();
  void sendEngineInfo();

}//namespace motors
#endif