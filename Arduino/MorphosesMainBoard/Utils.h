

// Flush serial.
void flushInputSerial(HardwareSerial& serial=Serial) {
  while (serial.available())
    serial.read();
}

// Wait until serial is ready.
void waitForInputSerial(HardwareSerial& serial=Serial) {
  while (!serial.available()) delay(10);
  flushInputSerial(serial);
}


void blinkIndicatorLed(unsigned long period, float pulseProportion=0.5, int nBlinks=1) {
  while (nBlinks--) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay((unsigned long) (period * pulseProportion));
    digitalWrite(LED_BUILTIN, LOW);
    delay((unsigned long) ((period * (1-pulseProportion))));
  }
}

// Compute firstAngle - secondAngle, remapped in [-180, 180]. The method that computes
// the angles should be called enough frequently that if the object rotate to the left
// or to the right it doesn't have enough time to rotate more than 180.
// Source: http://gmc.yoyogames.com/index.php?showtopic=386952
int getAngleDifference(int firstAngle, int secondAngle)
{
  return ((((secondAngle - firstAngle) % 360) + 540) % 360) - 180;
}

// Wraps an angle in degrees to be in [-180, 180].
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
