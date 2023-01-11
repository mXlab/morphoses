#include <SparkFun_BNO080_Arduino_Library.h>
#include <Chrono.h>


class MorphosesIMU : public BNO080 {
  private:
    boolean _isMain;
//    boolean _recalibrationMode;
    boolean _initialized;

    float _headingOffset;
  
  public:
    MorphosesIMU(boolean isMain) : _isMain(isMain), _initialized(false), _headingOffset(0) {}

    const char* name() const { return _isMain ? "main" : "side"; }

    // Start an IMU-specific OSC Message by calling this with appropriate sub-address. Result will be "/{main,side}/<addr>".
    OSCMessage& oscBundle(const char* addr) {
      char fullAddr[32];
      sprintf(fullAddr, "/%s%s", name(), addr);
      return bndl.add(fullAddr);
    }

    uint8_t i2cAddress() const { return _isMain ? 0x4B : 0x4A; }

    boolean isInitialized() const { return _initialized; }

    void init() {
      // Try to connect.
      boolean isOk = begin( i2cAddress() );
      if (!isOk) {
        Serial.println("BNO080 not detected at I2C address. Check your jumpers and the hookup guide. Freezing...");
        blinkIndicatorLed(1000, 0.1);
      }

      // Connected: initialize.
      else {
        Wire.setClock(400000); //Increase I2C data rate to 400kHz
    
        // Enable rotation vector.
        enableRotationVector(IMU_SAMPLE_RATE);
        enableMagnetometer(IMU_SAMPLE_RATE);
        
        _initialized = true;
      }
      
      // Add details and send.  
      oscBundle(isOk ? "/ready" : "/error").add(boardName).add("i2c");
      sendOscBundle();
    }

    boolean process() {
      // Get data.
      bool available = dataAvailable();
    
      // Send data over OSC.
      if (available && sendOSC)
      {
        oscBundle("/quat").add(getQuatI()).add(getQuatJ()).add(getQuatK()).add(getQuatReal());
        oscBundle("/rot").add((float)degrees(getRoll())).add((float)degrees(getPitch())).add((float)degrees(getYaw()));
        oscBundle("/accur").add(getMagAccuracy()).add(degrees(getQuatRadianAccuracy()));
        oscBundle("/mag").add(getMagX()).add(getMagY()).add(getMagZ());
      }
    
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

    void calibrateBegin() {
      calibrateAll();
      enableGameRotationVector(IMU_SAMPLE_RATE);
      enableMagnetometer(IMU_SAMPLE_RATE);
      oscBundle("/calibration-begin");
    }

    void calibrateEnd() {
      endCalibration();
      enableRotationVector(IMU_SAMPLE_RATE);
      enableMagnetometer(IMU_SAMPLE_RATE);
      oscBundle("/calibration-end");
    }

    void calibrateSave() {
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

    void tare(float currentHeading) {
      _headingOffset = currentHeading - getRawHeading();
    }

    float getHeading() {
      return wrapAngle180(getRawHeading() + _headingOffset);
    }

    float getRawHeading() {
      return (float)degrees(getYaw());
    }

};

MorphosesIMU imuMain(true);
MorphosesIMU imuSide(false);

#define IMU_LOW_ACCURACY  1
#define IMU_HIGH_ACCURACY 3

void initIMUs() {
  if (!imuMain.isInitialized())
    imuMain.init();
  if (!imuSide.isInitialized())
    imuSide.init();

  sendOscBundle();
}

void calibrateBeginIMUs() {
  imuMain.calibrateBegin();
  imuSide.calibrateBegin();
  sendOscBundle();
}

void calibrateEndIMUs() {
  imuMain.calibrateEnd();
  imuSide.calibrateEnd();  
  sendOscBundle();
}

void calibrateSaveIMUs() {
  imuMain.calibrateSave();
  imuSide.calibrateSave();  
  sendOscBundle();
}

void sleepIMUs() {
  imuMain.modeSleep();
  imuSide.modeSleep();
}

void wakeIMUs() {
  imuMain.modeOn();
  imuSide.modeOn();
}


void processIMUs() {
  imuMain.process();
  imuSide.process();
  sendOscBundle();
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
