#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_WATCHDOG_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_WATCHDOG_H_

#include "esp_task_wdt.h"

namespace watchdog {

  // Watchdog settings.
  const uint32_t TIMEOUT = 10; // Timeout in seconds.

  void initialize();
  void registerTask(TaskHandle_t taskHandle=NULL);
  void deleteCurrentTask();
  void reset();
}

#endif