/**
 * MorphosesIMUBOard
 * 
 * This program is part of the Morphoses project. It contains the code for 
 * the IMU ESP board which is located on the side of the ball inside the robot 
 * and is in charge of sending data about the rotation of the robot.
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
 #include "Config.h"

// Includes /////////////////////////////////////

// Configuration file.
#include "Config.h"
#include "Utils.h"
#include "OTA.h"
#include "Comm.h"
#include "IMU.h"
#include <Chrono.h>

// Variables & Objects //////////////////////////

Chrono sendDataChrono;

void setup()
{
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
}
