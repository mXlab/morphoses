#include <SparkFun_BNO080_Arduino_Library.h>

BNO080 imu;
#if DUAL_IMU
BNO080 imuSide;
#endif

boolean imuInitialized = false;

boolean imuIsInitialized() { 
  return imuInitialized; 
}

void initImu(BNO080& imu_, boolean i2cUseDefault) {
  uint8_t i2cAddr = i2cUseDefault ? 0x4B : 0x4A;
  if (!imu_.begin(i2cAddr)) {
    Serial.println("BNO080 not detected at I2C address. Check your jumpers and the hookup guide. Freezing...");
    bndl.add("/error");
    sendOscBundle();
    blinkIndicatorLed(1000, 0.1);
  }
  else {
    bndl.add("/ready");
    Wire.setClock(400000); //Increase I2C data rate to 400kHz
//    imu.calibrateAll();
    imu_.enableRotationVector(50); //Send data update every 50ms
    imu_.enableAccelerometer(50);

//    imu.enableGameRotationVector(50);
    imu_.enableMagnetometer(50);
    imuInitialized = true;
  }
  // Add details and send.
  bndl.add(boardName).add("imu-i2c").add(i2cUseDefault);
  sendOscBundle();
}

void initImu() {
  initImu(imu, true);
#if DUAL_IMU
  initImu(imuSide, false);
#endif
}

void sleepImu() {
  imu.modeSleep();
#if DUAL_IMU
  imuSide.modeSleep();
#endif
}

void wakeImu() {
  imu.modeOn();
#if DUAL_IMU
  imuSide.modeOn();
#endif
}

bool processImu(BNO080& imu_) {
  // Get data.
  bool dataAvailable = imu_.dataAvailable();

  // Send data over OSC.
  if (dataAvailable && sendOSC)
  {
    bndl.add("/quat").add(imu_.getQuatI()).add(imu_.getQuatJ()).add(imu_.getQuatK()).add(imu_.getQuatReal());
    bndl.add("/euler").add((float)degrees(imu_.getRoll())).add((float)degrees(imu_.getPitch())).add((float)degrees(imu_.getYaw()));
    bndl.add("/mag").add(imu_.getMagX()).add(imu_.getMagY()).add(imu_.getMagZ());
  }
  return dataAvailable;
}

bool processImu() {
  processImu(imu);
  sendOscBundle();
#if DUAL_IMU
  processImu(imuSide);
  sendOscBundle(false, false, false);
#endif
}
