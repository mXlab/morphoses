#include "Watchdog.h"



namespace watchdog {
  void initialize() {
    // Initialize watchdog timer in panic mode.
    esp_task_wdt_init(TIMEOUT, true);
  }

  void registerTask(TaskHandle_t taskHandle) {
    esp_task_wdt_add(taskHandle);
  }

  void reset() {
    // Reset watchdog timer.
    esp_task_wdt_reset();
  }

  void deleteCurrentTask(){
    esp_task_wdt_delete(NULL);
    }

}