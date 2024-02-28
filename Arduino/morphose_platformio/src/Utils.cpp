#include "Utils.h"
#include <Arduino.h>
#include "OSCMessage.h"

namespace utils {

void flushInputSerial(HardwareSerial& serial) {
  while (serial.available())
    serial.read();
}

void waitForInputSerial(HardwareSerial& serial) {
  while (!serial.available()) delay(10);
  flushInputSerial(serial);
}

void blinkIndicatorLed(unsigned long period, float pulseProportion, int nBlinks) {
  while (nBlinks--) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay((unsigned long) (period * pulseProportion));
    digitalWrite(LED_BUILTIN, LOW);
    delay((unsigned long) ((period * (1-pulseProportion))));
  }
}

int getAngleDifference(int firstAngle, int secondAngle)
{
  return ((((secondAngle - firstAngle) % 360) + 540) % 360) - 180;
}

float wrapAngle180(float angle) {
  while (angle >  180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

int safeRemapNorm(float unitVal, int maxRange, int midPoint) {
  float remappedVal = midPoint + constrain(unitVal, -1, 1) * maxRange;
  return round(remappedVal);
}

float wrapAngle360(float angle) {
  while (angle > 360) angle -= 360;
  while (angle <   0) angle += 360;
  return angle;
}




}  // namespace utils
