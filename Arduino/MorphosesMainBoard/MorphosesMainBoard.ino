/**
 * MorphosesMainBOard
 * 
 * This program is part of the Morphoses project. It contains the code for 
 * the MAIN ESP board which is located in the center of the robot and is in
 * charge of controlling the motors.
 * 
 * INSTRUCTIONS: Copy Config.h.default to Config.h and adjust according to 
 * your own Wifi setup.
 * 
 * (c) 2018-2021 Sofian Audry, Pierre Gaudet, Martin Peach
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
#include "Config.h"
#include "Utils.h"
#include "OTA.h"
#include "Comm.h"
#include "Motors.h"
#include "Pixels.h"
#include "IMU.h"
#include <Chrono.h>

// Variables & Objects //////////////////////////

Chrono sendDataChrono;

void setup()
{
  // Init neopixels.
  initPixels();

  // Init motors.
  initMotors();

  // Start I2C.
  Wire.begin();

  // Start serial.
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

 	// Initialize Wifi and UDP.
	initWifi();

  // Initialize OTA.
  initOTA(boardName);
}

void loop()
{
  // Update OTA.
  updateOTA();  
   
  // Check connection status: reconnect if connection lost.
  if (!wifiIsConnected())
    initWifi();

  // Init IMU if not already initialized.
  if (!imuIsInitialized())
    initImu();
  
  // Check for incoming messages.
  OSCMessage message;
  if (receiveMessage(message))
    processMessage(message);

  // Send messages.
  if (sendDataChrono.hasPassed(SEND_DATA_INTERVAL)) {
    sendData();
    sendDataChrono.restart();
  }
}

void sendData() {
  // Process motor ticks only if IMU ready.
  processImu();
//  if (processImu())
//    processMotors();
  if (DEBUG_MODE)
    sendMotorsInfo();

  // Send OSC bundle.
  sendOscBundle();
}

void processMessage(OSCMessage& messIn) {
  // This message assigns destination IP to the remote IP from which the OSC message was sent.
  if (messIn.fullMatch("/bonjour")) {
    if (DEBUG_MODE) Serial.println("Init IP");
    destIP = udp.remoteIP();
    bndl.add("/bonjour").add(boardName);
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
          wakeImu();
        else
          sleepImu();
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
      setMotorsPower( val != 0 );
    }
  }

  // Drive speed/pitch/forward-backward motor.
  else if (messIn.fullMatch("/speed")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("speed motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setMotorsSpeed(val);
    }
  }


  // Drive steer/tilt/left-right motor.
  else if (messIn.fullMatch("/steer")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("steer motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setMotorsSteer(val);
    }
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

}
