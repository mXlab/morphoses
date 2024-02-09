#ifndef MORPHOSE_IMU_H
#define MORPHOSE_IMU_H

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>

#include "communications/osc.h"

namespace imus{

class MorphosesIMU : public BNO080 {
  private:
    boolean _isMain;
//    boolean _recalibrationMode;
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

// Etienne : commented because only accessed by this file. could only be in .cpp
// MorphosesIMU imuMain(true);
// MorphosesIMU imuSide(false);

  #define IMU_LOW_ACCURACY  1
  #define IMU_HIGH_ACCURACY 3

  void initIMUs();
  void calibrateBeginIMUs();
  void calibrateEndIMUs();
  void calibrateSaveIMUs();
  void sleepIMUs();
  void wakeIMUs();
  void processIMUs();
  float getHeading();
  float getRawHeading();
  void tare(float currentHeading);

}//namespace imus

#endif