#define ENERGY_VOLTAGE_LOW_WAKEUP_TIME 10

#define ENERGY_VOLTAGE_OK_MODE       0
#define ENERGY_VOLTAGE_LOW_MODE      1
#define ENERGY_VOLTAGE_CRITICAL_MODE 2

//RTC_DATA_ATTR uint8_t energyVoltageMode   = ENERGY_VOLTAGE_OK_MODE;
//RTC_DATA_ATTR float   savedBatteryVoltage = -1;

void deepSleepLowMode(float batteryVoltage) {
  bndl.add("/error").add("battery-low").add(batteryVoltage);
  sendOscBundle();
  delay(1000);
      
  // Wakeup every 10 seconds.
  esp_sleep_enable_timer_wakeup(ENERGY_VOLTAGE_LOW_WAKEUP_TIME * 1000000UL);

  // Go to sleep.
  esp_deep_sleep_start(); 
}

void deepSleepCriticalMode(float batteryVoltage) {
  bndl.add("/error").add("battery-critical").add(batteryVoltage);
  sendOscBundle();
  delay(1000);

  // Go to sleep forever.
  esp_deep_sleep_start();
}

void checkEnergy() {

  // Read battery voltage.
  float batteryVoltage = getBatteryVoltage();
  
  // Low voltage: Launch safety procedure.
  if (batteryVoltage < ENERGY_VOLTAGE_LOW) {

    // Put IMUs to sleep to protect them.
    sleepIMUs();

    // Power engine off.
    setEnginePower(false);

    // If energy level is critical, just shut down the ESP.
    if (batteryVoltage < ENERGY_VOLTAGE_CRITICAL)
      deepSleepCriticalMode(batteryVoltage);

    // Otherwise, sleep but wake up to show that something is wrong.
    else
      deepSleepLowMode(batteryVoltage);
  }

}
