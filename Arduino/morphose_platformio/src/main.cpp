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
#include <PlaquetteLib.h>
using namespace pq;

#include <ArduinoLog.h>
//TODO : Merge config and global or move to Morphose.h
#include "Config.h"
#include "Globals.h"
#include "Morphose.h"

#include "Utils.h"
#include "communications/OTA.h"
#include "Comm.h"
#include "communications/Network.h"
#include "communications/osc.h"
#include "communications/MQTT.h"


// #include "Pixels.h"
// #include "Animation.h"
// #include "IMU.h"
// #include "Engine.h"
// #include "Navigation.h"
// #include "Energy.h"

//#include "OSCCallbacks.h"

#include <Chrono.h>

// Variables & Objects //////////////////////////

Chrono sendDataChrono;

//TaskHandle_t taskReceiveMessages;
//TaskHandle_t taskSendData;
//
//SemaphoreHandle_t receiveMessagesMutex = NULL;


// --------------- HELPERS ----------------

// void sendData() {
//   processIMUs();
//   processNavigation();
//   processEngine();

//   sendNavigationInfo();
//   sendEngineInfo();
  
//   // Send OSC bundle.
//   sendOscBundle();
// }

void processMessage() {
  
  // OSCMessage messIn;
  // IPAddress remote;
  // if (!receiveMessage(messIn, &remote))
  //   return;

  // // Print message.
  // if (DEBUG_MODE) {
  //   Serial.println(F("OSC message received:"));
  //   messIn.send(Serial);
  // }

  // using namespace osc_callback;
  // if (messIn.dispatch("/bonjour",bonjour));
  // if      (messIn.dispatch("/speed",   speed));
  // else if (messIn.dispatch("/steer",   steer));
  // else if (messIn.route("/nav",  navigation));

  // else if (messIn.dispatch("/reboot",  reboot));
  // else if (messIn.dispatch("/stream",  stream));
  // else if (messIn.dispatch("/power",  power));

  // else if (messIn.route("/calib",  calibration));
  // else if (messIn.route("/rgb", rgb));


//   else if (messIn.fullMatch("/base-color")) {
//    if (argIsNumber(messIn, 0) && argIsNumber(messIn, 1) && argIsNumber(messIn, 2)) { 
//      int r = getArgAsInt(messIn, 0);
//      int g = getArgAsInt(messIn, 1);
//      int b = getArgAsInt(messIn, 2);
////        int w = argIsNumber(messIn, 3) ? getArgAsInt(messIn, 3) : 0;
//      animation.baseColor.setRgb(r, g, b);
//    }
//  }
//
//   else if (messIn.fullMatch("/alt-color")) {
//    if (argIsNumber(messIn, 0) && argIsNumber(messIn, 1) && argIsNumber(messIn, 2)) { 
//      int r = getArgAsInt(messIn, 0);
//      int g = getArgAsInt(messIn, 1);
//      int b = getArgAsInt(messIn, 2);
////        int w = argIsNumber(messIn, 3) ? getArgAsInt(messIn, 3) : 0;
//      animation.altColor.setRgb(r, g, b);
//    }
//  }
//
//  else if (messIn.fullMatch("/period")) {
//    if (argIsNumber(messIn, 0)) {
//      float p = getArgAsFloat(messIn, 0);
//      animation.setPeriod(p);
//    }
//  }
//
//  else if (messIn.fullMatch("/noise")) {
//    if (argIsNumber(messIn, 0)) {
//      float noise = getArgAsFloat(messIn, 0);
//      int global = argIsNumber(messIn, 1) ? getArgAsBool(messIn, 1) : true;
//      animation.setNoise(noise, global);
//    }
//  }
//
//  else if (messIn.fullMatch("/animation-type")) {
//    if (argIsNumber(messIn, 0)) {
//      AnimationType type = (AnimationType) getArgAsInt(messIn, 0);
//      animation.setType(type);
//    }
//  }
//
//  else if (messIn.fullMatch("/animation-region")) {
//    if (argIsNumber(messIn, 0)) {
//      PixelRegion region = (PixelRegion) getArgAsInt(messIn, 0);
//      animation.setRegion(region);
//    }
//  }

}


// ---------- PROGRAM ---------------

void setup()
{
  delay(2000);
  Plaquette.begin();


//TODO : MOVE TO IMU
// // Start I2C.
//   Wire.begin();
//   Wire.setClock(400000); //Increase I2C data rate to 400kHz

  // Start serial.
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Log.begin(LOG_LEVEL_VERBOSE,&Serial);

  Log.infoln(" Morphose - 2023");



  // Initialize Wifi and UDP.

  //initWifi();
  network::initialize();

  morphose::initialize(network::getMcuIP());

  utils::debug(" OTA initialization");
  // Initialize OTA.
  initOTA(morphose::name); 

  // Initialize MQTT connection.
  utils::debug(" MQTT initialization");
  mqtt::initialize();



  // // Init motors.
  // debug(" Motors initialization");
  // initEngine();

  // // Check energy.
  // debug(" Energy initialization");
  // checkEnergy();

  // // Init neopixels.
  // debug(" LEDS initialization");
  // initPixels();

  // // Initialize animation system.
  // debug(" Animation initialization");
  // initAnimation();

  // debug(" IMU initialization");
  // initIMUs(); // maybe move to setup

  morphose::sayHello();
  utils::debug("---------------- End of setup ----------------");

 
}


void loop()
{

  // Check for incoming messages.
  // processMessage();

  // // Update OTA.
  updateOTA();

  // Check connection status: reconnect if connection lost.
  if (!network::isConnected()) {
    Serial.println("Lost wifi connection");
    network::initialize();
  }
  osc::update();

  // Init IMUs if not already initialized.
  // initIMUs(); // maybe move to setup

  // Update MQTT.
     mqtt::update();

    morphose::update();



  // // Send messages.
  // if (sendDataChrono.hasPassed(SEND_DATA_INTERVAL)) {
  //   //sendData();

  //   // Energy checkpoint to prevent damage when low 
  //   checkEnergy();

  //   // Restart chrono.
  //   sendDataChrono.start();
  // }
}

