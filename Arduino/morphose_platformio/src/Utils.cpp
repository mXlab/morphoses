#include "Utils.h"
#include <Arduino.h>

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

// int getAngleDifference(int firstAngle, int secondAngle)
// {
//   return ((((secondAngle - firstAngle) % 360) + 540) % 360) - 180;
// }

float getAngleDifference(float firstAngle, float secondAngle) {
  float diff = radians(firstAngle) - radians(secondAngle);
  float diffAngle = atan2(sin(diff), cos(diff));
  return degrees(diffAngle);
}


float wrapAngle180(float angle) {
  while (angle >  180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

float wrapAngle360(float angle) {
  while (angle > 360) angle -= 360;
  while (angle <   0) angle += 360;
  return angle;
}

int safeRemapNorm(float unitVal, int maxRange, int midPoint) {
  float remappedVal = midPoint + constrain(unitVal, -1, 1) * maxRange;
  return round(remappedVal);
}

const char* getResetReason() {
  return getResetReason(esp_reset_reason());
}

const char* getResetReason(esp_reset_reason_t resetReason) {
    switch (resetReason) {
        case ESP_RST_UNKNOWN:    return "Unknown reason";
        case ESP_RST_POWERON:    return "Power on reset";
        case ESP_RST_EXT:        return "External reset";
        case ESP_RST_SW:         return "Software reset";
        case ESP_RST_PANIC:      return "Software panic reset";
        case ESP_RST_INT_WDT:    return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT:   return "Task watchdog reset";
        case ESP_RST_WDT:        return "Other watchdog reset";
        case ESP_RST_DEEPSLEEP:  return "Wakeup from deep sleep";
        case ESP_RST_BROWNOUT:   return "Brownout reset";
        case ESP_RST_SDIO:       return "Reset over SDIO";
        default:                 return "Not defined";
    }
}

}  // namespace utils
