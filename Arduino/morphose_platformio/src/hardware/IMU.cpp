#include "IMU.h"

#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>

#include "communications/osc.h"
#include "Morphose.h"
#include "Utils.h"

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

    void MorphosesIMU::init() {

      // Try to connect.
      boolean isOk = begin(i2cAddress());
      if (!isOk) {
        osc::debug("BNO080 not detected at I2C address. Check your jumpers and the hookup guide. Freezing...");
        utils::blinkIndicatorLed(1000, 0.1);
        // Connected: initialize.
      } else {
        Wire.setClock(400000); //Increase I2C data rate to 400kHz
        // Enable rotation vector.
        enableRotationVector(IMU_SAMPLE_RATE);
        enableMagnetometer(IMU_SAMPLE_RATE);
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
      // if (available && osc::sendOSC) {  // flag that determines to send or not should be internal to class
      if (available) {
        oscBundle("/quat").add(getQuatI()).add(getQuatJ()).add(getQuatK()).add(getQuatReal());
        oscBundle("/rot").add((float)degrees(getRoll())).add((float)degrees(getPitch())).add((float)degrees(getYaw()));
        oscBundle("/accur").add(getMagAccuracy()).add(degrees(getQuatRadianAccuracy()));
        oscBundle("/mag").add(getMagX()).add(getMagY()).add(getMagZ());
      }

     // TODO(Etienne) : Old comment Verify if we keep
    //  // Verify if accuracy is okay, otherwise try to re-calibrate.
    //  if (getMagAccuracy() <= IMU_LOW_ACCURACY) {
    //    recalibrationMode = true;
    //
    //    while (getMagAccuracy() < IMU_HIGH_ACCURACY) {
    //      bndl.add("/error").add(boardName).add("accuracy").add(getMagAccuracy());
    //      sendOscBundle();
    //
    //      setMotorsPower(true);
    //      setMotorsSpeed(1);
    //
    //      setMotorsSteer(1);
    //      delay(10000UL);
    //      setMotorsSteer(-1);
    //      delay(10000UL);
    //    }
    //  }
      return available;
    }

    void MorphosesIMU::calibrateBegin() {
      calibrateAll();
      enableGameRotationVector(IMU_SAMPLE_RATE);
      enableMagnetometer(IMU_SAMPLE_RATE);
      oscBundle("/calibration-begin");
    }

    void MorphosesIMU::calibrateEnd() {
      endCalibration();
      enableRotationVector(IMU_SAMPLE_RATE);
      enableMagnetometer(IMU_SAMPLE_RATE);
      oscBundle("/calibration-end");
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
      oscBundle(isSaved ? "/calibration-save-done" : "/calibration-save-error");
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
  imuMain.calibrateBegin();
  imuSide.calibrateBegin();
  osc::sendBundle();
}

void endCalibration() {
  imuMain.calibrateEnd();
  imuSide.calibrateEnd();
  osc::sendBundle();
}

void saveCalibration() {
  imuMain.calibrateSave();
  imuSide.calibrateSave();
  osc::sendBundle();
}

void sleep() {
  imuMain.modeSleep();
  imuSide.modeSleep();
}

void wake() {
  imuMain.modeOn();
  imuSide.modeOn();
}


void process() {
  imuMain.process();
  imuSide.process();

  //osc::sendBundle(); // TODO(Etienne): Verify with Sofian why we are sending right away
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