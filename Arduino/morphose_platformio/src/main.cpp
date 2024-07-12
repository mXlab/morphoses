/**
 * MorphosesMainBoard
 * 
 * This program is part of the Morphoses project. It contains the code for 
 * the MAIN ESP board which is located in the center of the robot and is in
 * charge of controlling the motors.
 * 
 * Instructions 
 * ------------
 * 
 * 1. Copy Config.h.default to Config.h and adjust according to 
 *    your own Wifi setup.
 * 
 * 2. Install Arduino Library Dependencies: 
 *    Adafruit_NeoPixel, ArduinoOTA, Chrono, CNMAT_OSC, Dynamixel2Arduino, 
 *    SparkFun_BNO080_Arduino_Library, Plaquette, Adafruit_MQTT, Arduino_JSON,
 *    VectorXf
 *    
 * 3. ** IMPORTANT ** For uploading you need to use the board 
 *    "Adafruit ESP32 Feather". Do *NOT* use Sparkfun ESP32 Thing or Thing Plus
 *    otherwise you will have problems with reading the IMUs.
 *    
 * (c) 2018-2022 Sofian Audry, Pierre Gaudet, Martin Peach
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 *  /communications
 *   | osc
 *   | mqtt
 *   | ota
 *  
 *  /hardware 
 *   |imu
 *   |motor
 *   |pixel
 *   |energy (rename motor)
 * 
 *  /led
 *   |colors
 *   |animations
 */


#include <Arduino.h>

#include <Chrono.h>
#include <PlaquetteLib.h>
using namespace pq;

#include "communications/asyncMqtt.h"
#include "communications/Network.h"

#include "communications/OTA.h"

#include "hardware/Engine.h"
#include "hardware/IMU.h"

#include "lights/Animation.h"
#include "lights/Pixels.h"

// #include "Logger.h"
#include "Morphose.h"
#include "Utils.h"
#include "Watchdog.h"


void setup() {

  delay(5000);

  // Init watchdog.
  watchdog::initialize();
  // Register core task.
  watchdog::registerTask();

  // Launch everything.
  Plaquette.begin();
  Serial.begin(115200);

  Serial.println(" Morphose - 2024 - 11 ");

  Wire.begin();
  
  //logger::initialize();

  morphose::initialize();  // cannot send message before this point. Morphose needs to be initialized
 
  mqtt::initialize();
  mqtt::debug(" MQTT initialized");

  char buffer[128];
  sprintf(buffer, "Reset reason: %s\n", utils::getResetReason());
  mqtt::debug(buffer);

  motors::initialize();
  mqtt::debug("Motors initialized");

  pixels::initialize();
  mqtt::debug("LEDS initialized");

  animations::initialize();
  mqtt::debug(" Animation initialized");
  #if defined(MORPHOSE_DEBUG)
    animations::setDebugColor(DEBUG_COLOR_A, 0,0,200,0);
  #endif
  imus::initialize();
  mqtt::debug(" IMU initialized");

  // Initialize OTA.
  initOTA(morphose::name);
  mqtt::debug(" OTA initialized");

  morphose::energy::check();
  mqtt::debug("Energy initialized");

  mqtt::debug("---------------- End of setup ----------------");

  animations::setDebugColor(DEBUG_COLOR_A, 0,255,0,0);
  
}

void checkMemory() {
  static size_t lastAllocated = 0;

  // Get heap info.
  multi_heap_info_t memInfo;
  heap_caps_get_info(&memInfo, MALLOC_CAP_8BIT); 

  // Check if allocated memory increased.
  size_t currentAllocated = memInfo.total_allocated_bytes;
  if (currentAllocated > lastAllocated + 32) { // If memory usage increased by more than 32 bytes.
    // Print heap info.
    char buff[128];
    sprintf(buff, "Heap: %d allocated, %d free", memInfo.total_allocated_bytes, memInfo.total_free_bytes);

    mqtt::debug(buff);  
  }
  lastAllocated = currentAllocated;  


}

void loop() {
  // Ping watchdog.
  watchdog::reset();

 // logger::update();
  //logger::info("logger::update ok");

  // // Update OTA.
   updateOTA();
  

//  logger::info("mqtt::update ok");

   morphose::update();
// logger::info("morphose::update ok");

  //checkMemory();
}
