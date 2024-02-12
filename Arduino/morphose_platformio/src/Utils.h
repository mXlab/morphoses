#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_UTILS_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_UTILS_H_
#include <Arduino.h>

namespace utils {

// Flush serial.
void flushInputSerial(HardwareSerial& serial = Serial);

// Wait until serial is ready.
void waitForInputSerial(HardwareSerial& serial = Serial);

void blinkIndicatorLed(unsigned long period, float pulseProportion = 0.5, int nBlinks = 1);

// Compute firstAngle - secondAngle, remapped in [-180, 180]. The method that computes
// the angles should be called enough frequently that if the object rotate to the left
// or to the right it doesn't have enough time to rotate more than 180.
// Source: http://gmc.yoyogames.com/index.php?showtopic=386952
int getAngleDifference(int firstAngle, int secondAngle);

// Wraps an angle in degrees to be in [-180, 180].
float wrapAngle180(float angle);


// Remaps normalised value in [-1, 1] to [midPoint-maxRange, midPoint+maxRange].
int safeRemapNorm(float unitVal, int maxRange, int midPoint = 0);


float wrapAngle360(float angle);

void debug(const char *_msg);




}  // namespace utils


#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_UTILS_H_
