#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_


#include <Dynamixel2Arduino.h>
#include <Wire.h>

#include "Utils.h"

namespace motors {

  void initialize();
  void update();
  void setEnginePower(bool on);
  void setEngineSpeed(float speed);
  void setEngineSteer(float steer);
  float getEngineSpeed();
  float getEngineSteer();
  float engineIsMovingForward();
  float getBatteryVoltage();
  int getEngineSpeedTemperature();  
  int getEngineSteerTemperature();  
  void collectData();
  void dxlLibErrorToString(DXLLibErrorCode_t  error);
  void dxlPacketErrorToString(int  error);

}  // namespace motors
#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_

