

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
