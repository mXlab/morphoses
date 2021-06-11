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
#include "Motors.h"
#include "Pixels.h"

// WiFi & OSC.
#include <OSCBundle.h>
#include <WiFiUdp.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <ArduinoOTA.h>


// IMU.
#include <SparkFun_BNO080_Arduino_Library.h>

// Constants ////////////////////////////////////
#define WIFI_CONNECTION_TIMEOUT 5000

// Variables & Objects //////////////////////////

BNO080 imu;
OSCBundle bndl;
WiFiUDP udp;

IPAddress destIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, DEST_IP_3); // remote IP
IPAddress broadcastIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, 255); // broadcast

bool imuInitialized = false;
boolean sendOSC = true; // default

// Function declarations ////////////////////////
void sendOscBundle(boolean broadcast=false);
void blinkIndicatorLed(unsigned long period, float pulseProportion=0.5, int nBlinks=1);

void setup()
{
//  pinMode(power, OUTPUT);
//  digitalWrite(power, HIGH);

  initPixels();
  initMotors();

  Wire.begin();

  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(115200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  
//  // Set up the interrupt pin, it's set as active high, push-pull
//  pinMode(intPin, INPUT); // interrupt out from the IMU
//  digitalWrite(intPin, LOW);

	// Initialize Wifi and UDP.
	initWifi();

}

void loop()
{
   ArduinoOTA.handle();
   
  // Check connection status: reconnect if connection lost.
  if (WiFi.status() != WL_CONNECTED)
    initWifi();

  // Init IMU if not already initialized.
  if (!imuInitialized)
    initIMU();
  
  // Check for incoming messages.
  receiveMessage();

  // Send messages.
  sendData();
}

void receiveMessage() {
    // if there's data available, read a packet
  int packetSize = udp.parsePacket();

  if (packetSize)
  {
    if (DEBUG_MODE) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
    }
    IPAddress remote = udp.remoteIP();
    if (DEBUG_MODE) {
      for (int i = 0; i < 4; i++)
      {
        Serial.print(remote[i], DEC);
        if (i < 3)
        {
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(udp.remotePort());
    }

    // Read the packet
    OSCMessage messIn;
    while (packetSize--) messIn.fill(udp.read());

    switch(messIn.getError()) {
      case  OSC_OK:
        int messSize;
        if (DEBUG_MODE) Serial.println("no errors in packet");
        messSize = messIn.size();
        if (DEBUG_MODE) {
          Serial.print("messSize: ");
          Serial.println(messSize);
        }
        if (DEBUG_MODE) {
          char addressIn[64];
          messSize = messIn.getAddress(addressIn, 0, 64);
          Serial.print("messSize: ");
          Serial.println(messSize);
          Serial.print("address: ");
          Serial.println(addressIn);
        }
        processMessage(messIn);

        break;
      case BUFFER_FULL:
        if (DEBUG_MODE) Serial.println("BUFFER_FULL error");
        break;
      case INVALID_OSC:
        if (DEBUG_MODE) Serial.println("INVALID_OSC error");
        break;
      case ALLOCFAILED:
        if (DEBUG_MODE) Serial.println("ALLOCFAILED error");
        break;
      case INDEX_OUT_OF_BOUNDS:
        if (DEBUG_MODE) Serial.println("INDEX_OUT_OF_BOUNDS error");
        break;
    }
  } //if (packetSize)
}

void processMotors()
{
	// get the motor 1 encoder count
	byte incomingCount = Wire.requestFrom((uint8_t)MOTOR1_I2C_ADDRESS, (uint8_t)4);    // request 4 bytes from slave device #8
	byte tick3 = Wire.read();
	byte tick2 = Wire.read();
	byte tick1 = Wire.read();
	byte tick0 = Wire.read();
	if(DEBUG_MODE) {
	  Serial.print("received ");
	  Serial.print(incomingCount);
	  Serial.print(": (3)");
	  Serial.print(tick3, HEX);
	  Serial.print("(2)");
	  Serial.print(tick2, HEX);
	  Serial.print("(1)");
	  Serial.print(tick1, HEX);
	  Serial.print("(0)");
	  Serial.println(tick0, HEX);
	}
	int32_t motor1Ticks = (tick3<<24) + (tick2<<16) + (tick1<<8) + tick0;
	if (sendOSC) bndl.add("/motor/1/ticks").add(motor1Ticks);

	// get the motor 2 encoder count
	incomingCount = Wire.requestFrom((uint8_t)MOTOR2_I2C_ADDRESS, (uint8_t)4);    // request 4 bytes from slave device #16
	tick3 = Wire.read();
	tick2 = Wire.read();
	tick1 = Wire.read();
	tick0 = Wire.read();
	if(DEBUG_MODE) {
	  Serial.print("received ");
	  Serial.print(incomingCount);
	  Serial.print(": (3)");
	  Serial.print(tick3, HEX);
	  Serial.print("(2)");
	  Serial.print(tick2, HEX);
	  Serial.print("(1)");
	  Serial.print(tick1, HEX);
	  Serial.print("(0)");
	  Serial.println(tick0, HEX);
	}
	int32_t motor2Ticks = (tick3<<24) + (tick2<<16) + (tick1<<8) + tick0;
	if (sendOSC) bndl.add("/motor/2/ticks").add(motor2Ticks);
}

bool processImu() {
  // Get data.
  bool dataAvailable = imu.dataAvailable();

  // Send data over OSC.
  if (dataAvailable && sendOSC)
  {
    bndl.add("/quat").add(imu.getQuatI()).add(imu.getQuatJ()).add(imu.getQuatK()).add(imu.getQuatReal());
    bndl.add("/euler").add((float)degrees(imu.getRoll())).add((float)degrees(imu.getPitch())).add((float)degrees(imu.getYaw()));
//      bndl.add("/mag").add(imu.getMagX()).add(imu.getMagY()).add(imu.getMagZ());
  }
  return dataAvailable;
}

void sendData() {
  // Process motor ticks only if IMU ready.
  processImu();
//  if (processImu())
//    processMotors();

  // Send OSC bundle.
  sendOscBundle();
}

void initIMU() {
  if (!imuInitialized)
  {
    if (!imu.begin()) {
      Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
      bndl.add("/main/i2c/error");
      sendOscBundle();
      blinkIndicatorLed(1000, 0.1);
    }
    else {  
      bndl.add("/main/i2c/ok");
      sendOscBundle();
      Wire.setClock(400000); //Increase I2C data rate to 400kHz
      imu.enableRotationVector(50); //Send data update every 50ms
//      imu.enableMagnetometer(50);
      imuInitialized = true;
    }
  }
}

void initOTA() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("robot-1-main");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void initWifi()
{
  // now start the wifi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait for connection to complete.
  
  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startMillis < WIFI_CONNECTION_TIMEOUT) {
    Serial.print(".");
    blinkIndicatorLed(500);
  }

  // If still not connected, restart the board.
  if (WiFi.status() != WL_CONNECTED) {
    blinkIndicatorLed(100, 0.7, 20);
    ESP.restart();
  }

  initOTA();

  IPAddress myIP = WiFi.localIP();

  Serial.println("IP: ");
  Serial.println(myIP);

  if (!udp.begin(LOCAL_PORT)) {
    while(1); // Loop forever if setup didn't work
  }
  Serial.println("Done");

  // Broadcast IP address.
  bndl.add("/main/ip").add(myIP[3]);
  sendOscBundle(true);
}

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


/// Smart-converts argument from message to integer.
int32_t getArgAsInt(OSCMessage& msg, int index) {
  if (msg.isInt(index))
    return msg.getInt(index);
  else if (msg.isBoolean(index))
    return (msg.getBoolean(index) ? 1 : 0);
  else {
    double val = 0;
    if (msg.isFloat(index))       val = msg.getFloat(index);
    else if (msg.isDouble(index)) val = msg.getDouble(index);
    return round(val);
  }
}

float getArgAsFloat(OSCMessage& msg, int index) {
  if (msg.isFloat(index))
    return msg.getFloat(index);
  else if (msg.isDouble(index))
    return (float)msg.getDouble(index);
  else if (msg.isBoolean(index))
    return (msg.getBoolean(index) ? 1 : 0);
  else
    return (float)msg.getInt(index);
}

/// Returns true iff argument from message is convertible to a number.
boolean argIsNumber(OSCMessage& msg, int index) {
  return (msg.isInt(index) || msg.isFloat(index) || msg.isDouble(index) || msg.isBoolean(index));
}

void processMessage(OSCMessage& messIn) {
  // This message assigns destination IP to the remote IP from which the OSC message was sent.
  if (messIn.fullMatch("/bonjour")) {
    if (DEBUG_MODE) Serial.println("Init IP");
    destIP = udp.remoteIP();
    bndl.add("/bonjour").add(WiFi.localIP()[3]);
    sendOscBundle();
  }
  
  // Stream OSC messages ON/OFF.
  else if (messIn.fullMatch("/stream")) {
    if (DEBUG_MODE) Serial.println("STREAM");
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("stream value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      sendOSC = (val != 0);
    }
  }

  // Power motors ON/OFF.
  else if (messIn.fullMatch("/power")) {
    if (DEBUG_MODE) Serial.println("POWER");
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("power value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setMotorPower( val != 0 );
    }
  }

  // Drive speed/pitch/forward-backward motor.
  else if (messIn.fullMatch("/speed")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("speed motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setMotorSpeed(val);
    }
  }

  // Drive steer/tilt/left-right motor.
  else if (messIn.fullMatch("/steer")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("steer motor value ");
      float val = getArgAsFloat(messIn, 0);
      if (DEBUG_MODE) Serial.println(val);
      setMotorSteer(val);
    }
  }

  // Reset tilt motor to initial position.
  else if (messIn.fullMatch("/reset/2")) {
    // no args
    if (DEBUG_MODE) Serial.println("reset 2");
    Wire.beginTransmission(MOTOR2_I2C_ADDRESS); // transmit to device #8
    Wire.write(MOTOR_RESET); // sends one byte
    Wire.endTransmission(); // stop transmitting
  }

  else if (messIn.fullMatch("/rgb")) {
    if (argIsNumber(messIn, 0)) {
      if (DEBUG_MODE) Serial.print("RGB colors ");
      int r = getArgAsInt(messIn, 0);
      int g = getArgAsInt(messIn, 1);
      int b = getArgAsInt(messIn, 2);
      setPixels(r, b, g);
    }
  }

}

// Sends currently built bundle (with optional broadcasting option).
void sendOscBundle(boolean broadcast) {
  if (sendOSC) {
    udp.beginPacket(broadcast ? broadcastIP : destIP, DEST_PORT);
    bndl.send(udp); // send the bytes to the SLIP stream
    udp.endPacket(); // mark the end of the OSC Packet
  }
  bndl.empty(); // empty the bundle to free room for a new one
}

void blinkIndicatorLed(unsigned long period, float pulseProportion, int nBlinks) {
  while (nBlinks--) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay((unsigned long) (period * pulseProportion));
    digitalWrite(LED_BUILTIN, LOW);
    delay((unsigned long) ((period * (1-pulseProportion))));
  }
}
