#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>

#include "communications/osc.h"

namespace imus {

class MorphosesIMU : public BNO080 {
   private:
    boolean _isMain;
    boolean _initialized;

    float _headingOffset;

   public:
    MorphosesIMU(boolean isMain) : _isMain(isMain), _initialized(false), _headingOffset(0) {}

    const char* name() const;
    // Start an IMU-specific OSC Message by calling this with appropriate sub-address. Result will be "/{main,side}/<addr>".
    OSCMessage& oscBundle(const char* addr);

    uint8_t i2cAddress() const;
    boolean isInitialized() const;

    void init();

    boolean process();

    void calibrateBegin();

    void calibrateEnd();

    void calibrateSave();

    void tare(float currentHeading);

    float getHeading();

    float getRawHeading();
};


  void initialize();
  void beginCalibration();
  void endCalibration();
  void saveCalibration();
  void sleep();
  void wake();
  void process();
  float getHeading();
  float getRawHeading();  
  void tare(float currentHeading);

}  // namespace imus

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_
