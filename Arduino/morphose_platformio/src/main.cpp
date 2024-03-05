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
 *   | network
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




// Includes /////////////////////////////////////

// Configuration file.
#include <Arduino.h>

#include <ArduinoLog.h>
#include <Chrono.h>
#include <PlaquetteLib.h>
using namespace pq;

#include "communications/MQTT.h"
#include "communications/Network.h"
#include "communications/osc.h"
#include "communications/OTA.h"

#include "hardware/Engine.h"
#include "hardware/IMU.h"

#include "lights/Animation.h"
#include "lights/Pixels.h"

#include "Logger.h"
#include "Morphose.h"
#include "Utils.h"


// Variables & Objects //////////////////////////

Chrono sendDataChrono;


// ---------- PROGRAM ---------------

void setup() {

  delay(5000);
  Plaquette.begin();
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.infoln(" Morphose - 2023 - 6");
  Wire.begin();
  
  logger::initialize();

  network::initialize();
  delay(1000);
  osc::debug("Network initialized");
  char buff[32];
  sprintf(buff,"Osc broadcast state: %d\n",  osc::isBroadcasting());

  osc::debug(buff);
  motors::initialize();
  osc::debug("Motors initialized");

  morphose::energy::check();
  osc::debug("Energy initialized");

  pixels::initialize();
  osc::debug("LEDS initialized");

 animations::initialize();
  osc::debug(" Animation initialized");

  imus::initialize();
  osc::debug(" IMU initialized");

  morphose::initialize(network::mcuIP);  // cannot send message before this point. Morphose needs to be initialized

  mqtt::initialize();
  osc::debug(" MQTT initialized");

 
  // Initialize OTA.
  // initOTA(morphose::name);
  osc::debug(" OTA initialized");

  morphose::sayHello();
  osc::debug("---------------- End of setup ----------------");

  logger::info("Morphose initialization ok");
}


void loop() {
  logger::update();
  // // Update OTA.
  // updateOTA();
  imus::initialize();
  mqtt::update();
  morphose::update();
  osc::update();

}

