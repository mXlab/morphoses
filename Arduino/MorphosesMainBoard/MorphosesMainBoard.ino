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
 *    SparkFun_BNO080_Arduino_Library
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

// Includes /////////////////////////////////////

// Configuration file.
#include <PlaquetteLib.h>
using namespace pq;

#include "Config.h"
#include "Globals.h"
#include "Utils.h"
#include "OTA.h"
#include "Comm.h"
#include "Location.h"
#include "IMU.h"
#include "Engine.h"
#include "Pixels.h"

// Variables & Objects //////////////////////////

Metro sendDataMetro(SEND_DATA_INTERVAL / 1000.0f);


void setup()
{
  Plaquette.begin();
  
  // Init neopixels.
  initPixels();

  // Init motors.
  initEngine();

  // Start I2C.
  Wire.begin();

  // Start serial.
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

 	// Initialize Wifi and UDP.
	initWifi();

  initMqtt();
  
  // Initialize OTA.
  initOTA(boardName);
}

void loop()
{
  Plaquette.step();

  // Update OTA.
  updateOTA();
   
  // Check connection status: reconnect if connection lost.
  if (!wifiIsConnected())
    initWifi();

  // Init IMUs if not already initialized.
  initIMUs();

  // 
  updateMqtt();
  
  // Check for incoming messages.
  processMessage();

  // Send messages.
  if (sendDataMetro) {
    sendData();
  }
}

void sendData() {
  // Process motor ticks only if IMU ready.
  processIMUs();
//  if (processImu())
//    processMotors();
  if (DEBUG_MODE)
    sendEngineInfo();

  processEngine();
  
  // Send OSC bundle.
  sendOscBundle();
}

void processMessage() {
  
  OSCMessage messIn;
  IPAddress remote;
  if (!receiveMessage(messIn, &remote))
    return;

  // This message assigns destination IP to the remote IP from which the OSC message was sent.
  if (messIn.fullMatch("/bonjour")) {
    if (DEBUG_MODE) Serial.println("Init IP");
    // Collect last part of IP address and add it to the list of destinations.
    byte destIP3 = argIsNumber(messIn, 0) ? getArgAsInt(messIn, 0) : remote[3];
    addDestinationIPAddress(destIP3);
    bndl.add("/bonjour").add(boardName).add(destIP3);
    sendOscBundle();
  }

  // Reboot the ESP.
  else if (messIn.fullMatch("/reboot")) {
    ESP.restart();
  }

  // Stream OSC messages ON/OFF.
  else if (messIn.fullMatch("/stream")) {
    if (DEBUG_MODE) Serial.println("STREAM");
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("stream value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      boolean stream = (val != 0);
      // If new value, enable/disable OSC & wake/sleep IMU.
      if (sendOSC != stream) {
        sendOSC = stream;
        if (sendOSC)
          wakeIMUs();
        else
          sleepIMUs();
      }
    }
  }

  // Power motors ON/OFF.
  else if (messIn.fullMatch("/power")) {
    if (DEBUG_MODE) Serial.println("POWER");
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("power value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setEnginePower( val != 0 );
    }
  }

  // Drive speed/pitch/forward-backward motor.
  else if (messIn.fullMatch("/speed")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("speed motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setEngineSpeed(val);
    }
  }


  // Drive steer/tilt/left-right motor.
  else if (messIn.fullMatch("/steer")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("steer motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setEngineSteer(val);
    }
  }

  // Head in a certain direction.
  else if (messIn.fullMatch("/heading-start")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("start heading ");
      float speed = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(speed);
      startEngineHeading(speed, argIsNumber(messIn, 1) ? getArgAsFloat(messIn, 1) : 0);
    }
  }

  // Head in a certain direction.
  else if (messIn.fullMatch("/heading-stop")) {
    stopEngineHeading();
  }

  else if (messIn.fullMatch("/rgb")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("RGB colors ");
      if (argIsNumber(messIn, 0) && argIsNumber(messIn, 1) && argIsNumber(messIn, 2)) { 
        int r = getArgAsInt(messIn, 0);
        int g = getArgAsInt(messIn, 1);
        int b = getArgAsInt(messIn, 2);
        int w = argIsNumber(messIn, 3) ? getArgAsInt(messIn, 3) : 0;
        if (DEBUG_MODE) { Serial.print(r); Serial.print(" "); Serial.print(g); Serial.print(" "); Serial.println(b); }
        setPixels(r, b, g, w);
      }
    }
  }

  else if (messIn.fullMatch("/rgb-one")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("RGB colors ");
      if (argIsNumber(messIn, 0) && argIsNumber(messIn, 1) && argIsNumber(messIn, 2) && argIsNumber(messIn, 3)) { 
        int i = getArgAsInt(messIn, 0);
        int r = getArgAsInt(messIn, 1);
        int g = getArgAsInt(messIn, 2);
        int b = getArgAsInt(messIn, 3);
        int w = argIsNumber(messIn, 4) ? getArgAsInt(messIn, 4) : 0;
        if (DEBUG_MODE) { Serial.print(r); Serial.print(" "); Serial.print(g); Serial.print(" "); Serial.println(b); }
        setPixel(i, r, b, g, w);
      }
    }
  }

  else if (messIn.fullMatch("/rgb-region")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("RGB colors ");
      if (argIsNumber(messIn, 0) && argIsNumber(messIn, 1) && argIsNumber(messIn, 2) && argIsNumber(messIn, 3)) { 
        PixelRegion region = (PixelRegion) getArgAsInt(messIn, 0);
        int r = getArgAsInt(messIn, 1);
        int g = getArgAsInt(messIn, 2);
        int b = getArgAsInt(messIn, 3);
        int w = argIsNumber(messIn, 4) ? getArgAsInt(messIn, 4) : 0;
        if (DEBUG_MODE) { Serial.print(r); Serial.print(" "); Serial.print(g); Serial.print(" "); Serial.println(b); }
        setPixelsRegion(region, r, b, g, w);
      }
    }
  }

  else if (messIn.fullMatch("/calibrate-begin")) {
    calibrateBeginIMUs();
  }

  else if (messIn.fullMatch("/calibrate-end")) {
    calibrateEndIMUs();
  }

  else if (messIn.fullMatch("/calibrate-save")) {
    calibrateSaveIMUs();
  }

}
