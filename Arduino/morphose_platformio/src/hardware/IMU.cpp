#include "IMU.h"

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>


#include "communications/asyncMqtt.h"
#include "Morphose.h"
#include "Utils.h"
#include "lights/Animation.h"

namespace imus {

  const char* MorphosesIMU::name() const { return _isMain ? "main" : "side"; }

  // Start an IMU-specific OSC Message by calling this with appropriate sub-address. Result will be "/{main,side}/<addr>".
  // OSCMessage& MorphosesIMU::oscBundle(const char* addr) {
  //   char fullAddr[32];
  //   sprintf(fullAddr, "/%s%s", name(), addr);
  //   return osc::bundle.add(fullAddr);
  // }

    uint8_t MorphosesIMU::i2cAddress() const { return _isMain ? 0x4B : 0x4A; }

    boolean MorphosesIMU::isInitialized() const { return _initialized; }

    void MorphosesIMU::_enableSensors() {
        enableRotationVector(IMU_SAMPLE_RATE);
        enableMagnetometer(IMU_SAMPLE_RATE);
    }

    void MorphosesIMU::init() {

      // Try to connect.
      boolean isOk = begin(i2cAddress());
      if (!isOk) {
        //logger::error("Cant initialize imus");
        mqtt::debug("BNO080 not detected at I2C address. Check your jumpers and the hookup guide. Freezing...");
//        utils::blinkIndicatorLed(1000, 0.1);
      } else {
        // Increase I2C data rate to 400kHz.
        Wire.setClock(400000); 
        // Enable sensors.
        _enableSensors();
        // Mark as initialied.
        _initialized = true;
         mqtt::debug("BNO080 initialized");
      }

      // Add details and send.
      mqtt::debug(name());
      mqtt::debug(isOk ? "is ready" : " error");
      mqtt::debug(isOk ? "is ready" : " error");
    }

    boolean MorphosesIMU::process() {
      // Get data.
      bool available = dataAvailable();

      // Send data over OSC.
      if (available) {
        // Store rotation.
        _rot[0].add(degrees(getRoll()));
        _rot[1].add(degrees(getPitch()));
        _rot[2].add(degrees(getYaw()));

        // Store quaternion.
        _quat[0].add(getQuatI());
        _quat[1].add(getQuatJ());
        _quat[2].add(getQuatK());
        _quat[3].add(getQuatReal());

      }
      else {
        // Just step.
        for (int i=0; i<3; i++)
          _rot[i].addEmpty();
        for (int i=0; i<4; i++)
          _quat[i].addEmpty();
      }

      return available;
    }

    void MorphosesIMU::collectData() {
        // Debugging info //////////////////////////////////////////

        // Add quaternion.
        JsonObject imuJson = morphose::json::deviceData.createNestedObject(name());
        JsonArray quatData = imuJson["quat"].to<JsonArray>();
        JsonArray dQuatData = imuJson["d-quat"].to<JsonArray>();
        // Add quaternion.
        for (int i=0; i<4; i++) {
            quatData.add(_quat[i].value());
            dQuatData.add(_quat[i].delta());
        }
    
        // Add rotation.
        JsonArray rotData = imuJson["rot"].to<JsonArray>();
        JsonArray dRotData = imuJson["d-rot"].to<JsonArray>();
        for (int i=0; i<3; i++) {
            rotData.add(_rot[i].value());
            dRotData.add(_rot[i].delta());
        }

        // Add magnetometer.
        JsonArray magData = imuJson["mag"].to<JsonArray>();
        magData.add(getMagX());
        magData.add(getMagY());
        magData.add(getMagZ());

        // Add accuracy.
        imuJson["accur-mag"] = getMagAccuracy();
        imuJson["accur-quat"] = degrees(getQuatRadianAccuracy());
    }

    void MorphosesIMU::calibrateBegin() {
      calibrateAll();
      _enableSensors();
        mqtt::debug("Calibration begin");
    }

    void MorphosesIMU::calibrateEnd() {
      endCalibration();
      _enableSensors();
        mqtt::debug("Calibration end");
    }

    void MorphosesIMU::calibrateSave() {
      saveCalibration();
      requestCalibrationStatus();
      Chrono calibrationChrono;
      calibrationChrono.start();
      bool isSaved = false;
      while (!calibrationChrono.hasPassed(100)) {
        if (dataAvailable() && calibrationComplete()) {
          isSaved = true;
          break;
        }
      }

      // Send feedback.
      mqtt::debug(isSaved ? "/calibration-save-done" : "/calibration-save-error");
    }

    void MorphosesIMU::tare(float currentHeading) {
      _headingOffset = currentHeading - getRawHeading();
    }

    float MorphosesIMU::getHeading() {
      return utils::wrapAngle180(getRawHeading() + _headingOffset);
    }

    float MorphosesIMU::getRawHeading() {
      return (float)degrees(getYaw());
    }

///----

MorphosesIMU imuMain(true);
MorphosesIMU imuSide(false);

#define IMU_LOW_ACCURACY  1
#define IMU_HIGH_ACCURACY 3

void initialize() {
    
  if (!imuMain.isInitialized()) {
    
    mqtt::debug("Main imu not initialized");
    #if defined(MORPHOSE_DEBUG)
      animations::setDebugColor(DEBUG_COLOR_A,255,0,0,0);
    #endif
    imuMain.init();
  }

  if (!imuSide.isInitialized()) {
    mqtt::debug("Side imu not initialized");
    #if defined(MORPHOSE_DEBUG)
      animations::setDebugColor(DEBUG_COLOR_A,10,0,0,0);
    #endif
    imuSide.init();
  }

}

void beginCalibration() {
  mqtt::debug("Begin calibration");
  imuMain.calibrateBegin();
  imuSide.calibrateBegin();
}

void endCalibration() {
  mqtt::debug("End calibration");
  imuMain.calibrateEnd();
  imuSide.calibrateEnd();
}

void saveCalibration() {
  imuMain.calibrateSave();
  imuSide.calibrateSave();
}

void sleep() {
  mqtt::debug("imus Sleeping");
  imuMain.modeSleep();
  imuSide.modeSleep();
}

void wake() {
  mqtt::debug("imus Waking");
  imuMain.modeOn();
  imuSide.modeOn();
}


void process() {
  mqtt::debug("imus Process");
  #if defined(MORPHOSE_DEBUG)
  animations::setDebugColor(DEBUG_COLOR_A,0,50,0,0);
  #endif
  imuMain.process();
  imuSide.process();
  #if defined(MORPHOSE_DEBUG)
  animations::setDebugColor(DEBUG_COLOR_A,0,0,100,0);
  #endif
}

void collectData() {
  imuMain.collectData();
  imuSide.collectData();
}

float getHeading() {
  return imuMain.getHeading();
}

float getRawHeading() {
  return imuMain.getRawHeading();
}

void tare(float currentHeading) {
  imuMain.tare(currentHeading);
}

}  // namespace imus
