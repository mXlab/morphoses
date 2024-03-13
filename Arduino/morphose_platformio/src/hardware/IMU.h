#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>

#include "Utils.h"
#include "communications/osc.h"

namespace imus {

#define DATA_ITEM_SMOOTHING_WINDOW 0.1f // s
#define STREAM_FREQUENCY (1000.0f / STREAM_INTERVAL) // Hz
const float DATA_ITEM_ALPHA = 1.0f / (STREAM_FREQUENCY * DATA_ITEM_SMOOTHING_WINDOW);

class DataItem {
   public:
    DataItem(bool isAngle=false)
        : _value(0), _delta(0), _isAngle(isAngle), _isInitialized(false) {}

    void add(float value) {
        if (isnan(value))
          return;
        // Compute delta.
        float delta = _isAngle ? utils::getAngleDifference(value, _value) : (value - _value);
        delta *= STREAM_FREQUENCY; // Convert to variation/s.

        // Update values using moving average.
        if (_isInitialized) {
          _value -= DATA_ITEM_ALPHA * (_value - value);
          _delta -= DATA_ITEM_ALPHA * (_delta - delta);
        }

        // Initialize: use argument as unbiased initial value.
        else {
          _value = value;
          _delta = delta;
          _isInitialized = true;
        }
    }

    // Just add the current value to the moving average.
    void addEmpty() {
      if (_isInitialized)
        add(_value);
    }

    float value() const { return _value; }
    float delta() const { return _delta; }

   private:
    float _value;
    float _delta;
    bool _isAngle : 4;
    bool _isInitialized : 4;

};

class MorphosesIMU : public BNO080 {
   private:
    boolean _isMain;
    boolean _initialized;

    float _headingOffset;

    DataItem _rot[3] = { true, true, true };
    DataItem _quat[4];

    void _enableSensors();

   public:
    // Constructor.
    MorphosesIMU(boolean isMain) : _isMain(isMain), _initialized(false), _headingOffset(0) {}

    // Return IMU name.
    const char* name() const;

    // Start an IMU-specific OSC Message by calling this with appropriate sub-address. Result will be "/{main,side}/<addr>".
    OSCMessage& oscBundle(const char* addr);

    // Return I2C address.
    uint8_t i2cAddress() const;

    // Returns true iff initialized.
    boolean isInitialized() const;

    // Initializes IMU.
    void init();

    // Gets data and store it.
    boolean process();

    // Sends stored data as OSC.
    void sendData();

    // Calibration functions.
    void calibrateBegin();
    void calibrateEnd();
    void calibrateSave();

    // Sets tare to current heading.
    void tare(float currentHeading);

    // Gets heading.
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
  void sendData();
  void tare(float currentHeading);

}  // namespace imus

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_HARDWARE_IMU_H_
