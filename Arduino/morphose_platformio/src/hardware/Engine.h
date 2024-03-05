#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_


#include <Dynamixel2Arduino.h>
#include <Wire.h>

#include "Utils.h"

namespace motors {

  void initialize();
  void setEnginePower(bool on);
  void setEngineSpeed(float speed);
  void setEngineSteer(float steer);
  float getEngineSpeed();
  float getEngineSteer();
  float engineIsMovingForward();
  float getBatteryVoltage();
  int getEngineSpeedTemperature();  // TODO(Etienne): Never used. Verify if need to keep
  int getEngineSteerTemperature();  // TODO(Etienne): Never used. Verify if need to keep
  void sendEngineInfo();

}  // namespace motors
#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_ENGINE_H_
