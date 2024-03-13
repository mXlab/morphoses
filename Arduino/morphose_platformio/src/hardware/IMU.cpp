#include "IMU.h"

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>

#include "communications/osc.h"
#include "Morphose.h"
#include "Utils.h"
#include "Logger.h"

namespace imus {


  const char* MorphosesIMU::name() const { return _isMain ? "main" : "side"; }

  // Start an IMU-specific OSC Message by calling this with appropriate sub-address. Result will be "/{main,side}/<addr>".
  OSCMessage& MorphosesIMU::oscBundle(const char* addr) {
    char fullAddr[32];
    sprintf(fullAddr, "/%s%s", name(), addr);
    return osc::bundle.add(fullAddr);
  }

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
        logger::error("Cant initialize imus");
        osc::debug("BNO080 not detected at I2C address. Check your jumpers and the hookup guide. Freezing...");
//        utils::blinkIndicatorLed(1000, 0.1);
      } else {
        // Increase I2C data rate to 400kHz.
        Wire.setClock(400000); 
        // Enable sensors.
        _enableSensors();
        // Mark as initialied.
        _initialized = true;
         osc::debug("BNO080 initialized");
      }

      // Add details and send.
      oscBundle(isOk ? "/ready" : "/error").add(morphose::name).add("i2c");
      osc::sendBundle();
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

        // oscBundle("/quat").add(getQuatI()).add(getQuatJ()).add(getQuatK()).add(getQuatReal());
        // oscBundle("/rot").add((float)degrees(getRoll())).add((float)degrees(getPitch())).add((float)degrees(getYaw()));
        // oscBundle("/accur").add(getMagAccuracy()).add(degrees(getQuatRadianAccuracy()));
        // oscBundle("/mag").add(getMagX()).add(getMagY()).add(getMagZ());
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

    void MorphosesIMU::sendData() {
        // Debugging info //////////////////////////////////////////

        JsonArray quatData = morphose::json::deviceData["quat"].to<JsonArray>();
        JsonArray dQuatData = morphose::json::deviceData["d-quat"].to<JsonArray>();
        // Add quaternion.
        for (int i=0; i<4; i++) {
            quatData.add(_quat[i].value());
            dQuatData.add(_quat[i].delta());
        }
    
        // Add rotation.
        JsonArray rotData = morphose::json::deviceData["rot"].to<JsonArray>();
        JsonArray dRotData = morphose::json::deviceData["d-rot"].to<JsonArray>();
        for (int i=0; i<3; i++) {
            rotData.add(_rot[i].value());
            dRotData.add(_rot[i].delta());
        }

        // Add magnetometer.
        JsonArray magData = morphose::json::deviceData["mag"].to<JsonArray>();
        magData.add(getMagX());
        magData.add(getMagY());
        magData.add(getMagZ());

        // Useful info ////////////////////////////////////////////

        // Add full data bundle.
        // MQTT_JSON refactor, still needed?
        // OSCMessage& msgFull = oscBundle("/data");
        // for (int i=0; i<4; i++) msgFull.add(_quat[i].value()); // quaternion
        // for (int i=0; i<4; i++) msgFull.add(_quat[i].delta()); // delta quaternion
        // for (int i=0; i<3; i++) msgFull.add(_rot[i].value());   // rotation
        // for (int i=0; i<3; i++) msgFull.add(_rot[i].delta());   // delta rotation

        // Add accuracy.
        morphose::json::deviceData["accur-mag"] = getMagAccuracy();
        morphose::json::deviceData["accur-quat"] = degrees(getQuatRadianAccuracy());
    }

    void MorphosesIMU::calibrateBegin() {
      calibrateAll();
      _enableSensors();
        osc::debug("Calibration begin");
    }

    void MorphosesIMU::calibrateEnd() {
      endCalibration();
      _enableSensors();
        osc::debug("Calibration end");
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
      osc::debug(isSaved ? "/calibration-save-done" : "/calibration-save-error");
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
    osc::debug("Main imu not initialized");
    imuMain.init();
  }

  if (!imuSide.isInitialized()) {
    osc::debug("Side imu not initialized");
    imuSide.init();
  }

}

void beginCalibration() {
  osc::debug("Begin calibration");
  imuMain.calibrateBegin();
  imuSide.calibrateBegin();
}

void endCalibration() {
  osc::debug("End calibration");
  imuMain.calibrateEnd();
  imuSide.calibrateEnd();
}

void saveCalibration() {
  imuMain.calibrateSave();
  imuSide.calibrateSave();
}

void sleep() {
  osc::debug("imus Sleeping");
  imuMain.modeSleep();
  imuSide.modeSleep();
}

void wake() {
  osc::debug("imus Waking");
  imuMain.modeOn();
  imuSide.modeOn();
}


void process() {
  osc::debug("imus Process");
  imuMain.process();
  imuSide.process();
}

void sendData() {
  imuMain.sendData();
  imuSide.sendData();
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
