#include <SparkFun_BNO080_Arduino_Library.h>

BNO080 imu;

boolean imuInitialized = false;

boolean imuIsInitialized() { 
  return imuInitialized; 
}

void initImu() {
  if (!imu.begin()) {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    bndl.add("/error");
    sendOscBundle();
    blinkIndicatorLed(1000, 0.1);
  }
  else {
    bndl.add("/ready");
    Wire.setClock(400000); //Increase I2C data rate to 400kHz
    imu.enableRotationVector(50); //Send data update every 50ms
//      imu.enableMagnetometer(50);
    imu.calibrateAll();
    imuInitialized = true;
  }
  // Add details and send.
  bndl.add(boardName).add("imu-i2c");
  sendOscBundle();
}

void sleepImu() {
  imu.modeSleep();
}

void wakeImu() {
  imu.modeOn();
}

bool processImu() {
  // Get data.
  bool dataAvailable = imu.dataAvailable();

  // Send data over OSC.
  if (dataAvailable && sendOSC)
  {
    bndl.add("/quat").add(imu.getQuatI()).add(imu.getQuatJ()).add(imu.getQuatK()).add(imu.getQuatReal());
//    bndl.add("/euler").add((float)degrees(imu.getRoll())).add((float)degrees(imu.getPitch())).add((float)degrees(imu.getYaw()));
//      bndl.add("/mag").add(imu.getMagX()).add(imu.getMagY()).add(imu.getMagZ());
  }
  return dataAvailable;
}
