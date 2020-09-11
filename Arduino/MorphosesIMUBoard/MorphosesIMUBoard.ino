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
 * (c) 2018-2020 Sofian Audry, Martin Peach
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

 #include <driver/adc.h>

#include <SparkFun_BNO080_Arduino_Library.h>

#include <Chrono.h>

#include <OSCBundle.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiUdp.h>
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);

#define WIFI_CONNECTION_TIMEOUT 5000

BNO080 imu;
WiFiUDP udp;
OSCBundle bndl;

IPAddress destIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, DEST_IP_3); // remote IP
IPAddress broadcastIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, 255); // broadcast

char packetBuffer[128];

void sendOscBundle(boolean broadcast=false);
void blinkIndicatorLed(unsigned long period, float pulseProportion=0.5, int nBlinks=1);

bool imuInitialized = false;

//int sensorValue = 0;
//int analogPin = 4;

// Chronometer to control when the IMU data is fetched.
Chrono imuDataChrono;

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize Wifi and UDP.
  initWifi();

  Wire.begin();

  digitalWrite(LED_BUILTIN, HIGH);

  //adc1_config_width(ADC_WIDTH_BIT_12);
  //adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
  //float value = (float)adc1_get_raw(ADC1_CHANNEL_0);
}

void loop() {
  //sensorValue = analogRead(analogPin);
  //Serial.println(sensorValue);
  
  //int value = adc1_get_raw(ADC1_CHANNEL_0);
  //Serial.println(value);
  //delay(50);
  
  // Check connection status: reconnect if connection lost.
  if (WiFi.status() != WL_CONNECTED)
    initWifi();

  // Init IMU if not already initialized.
  if (!imuInitialized)
    initIMU();
  
  // Check for incoming messages.
  receiveMessage();
  
  // Send IMU.
  if (imuDataChrono.hasPassed(SEND_DATA_INTERVAL)) {
    imuDataChrono.restart();
    processImu();
  }
}

void receiveMessage() {
  // if there's data available, read a packet
  int packetSize = udp.parsePacket();

  if (packetSize)
  {
    if (OSCDebug) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
    }
    IPAddress remote = udp.remoteIP();
    if (OSCDebug) {
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
        if (OSCDebug) Serial.println("no errors in packet");
        messSize = messIn.size();
        if (SerialDebug) {
          Serial.print("messSize: ");
          Serial.println(messSize);
        }
        if (OSCDebug) {
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
        if (OSCDebug) Serial.println("BUFFER_FULL error");
        break;
      case INVALID_OSC:
        if (OSCDebug) Serial.println("INVALID_OSC error");
        break;
      case ALLOCFAILED:
        if (OSCDebug) Serial.println("ALLOCFAILED error");
        break;
      case INDEX_OUT_OF_BOUNDS:
        if (OSCDebug) Serial.println("INDEX_OUT_OF_BOUNDS error");
        break;
    }
  } //if (packetSize)
}

void processMessage(OSCMessage& messIn) {
  // This message assigns destination IP to the remote IP from which the OSC message was sent.
  if (messIn.fullMatch("/bonjour")) {
    if (OSCDebug) Serial.println("Init IP");
    destIP = udp.remoteIP();
    bndl.add("/imu/bonjour");
    sendOscBundle();
  }
}

void processImu() {
  if (imu.dataAvailable() == true)
  {
//    float quatI = imu.getQuatI();
//    float quatJ = imu.getQuatJ();
//    float quatK = imu.getQuatK();
//    float quatReal = imu.getQuatReal();
//    Serial.println(quatI);
//    Serial.println(quatJ);
//    Serial.println(quatK);
//    Serial.println(quatReal);
////    float quatRadianAccuracy = imu.getQuatRadianAccuracy();

    if (sendOSC) {
      bndl.add("/quat").add(imu.getQuatI()).add(imu.getQuatJ()).add(imu.getQuatK()).add(imu.getQuatReal());
      bndl.add("/euler").add((float)degrees(imu.getRoll())).add((float)degrees(imu.getPitch())).add((float)degrees(imu.getYaw()));
      sendOscBundle();
    }
  }
}

void initIMU() {
  if (!imuInitialized)
  {
    if (!imu.begin()) {
      Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
      bndl.add("/imu/i2c/error");
      sendOscBundle();
      blinkIndicatorLed(1000, 0.1);
    }
    else {  
      bndl.add("/imu/i2c/ok");
      sendOscBundle();
      Wire.setClock(400000); //Increase I2C data rate to 400kHz
      imu.enableRotationVector(50); //Send data update every 50ms
      imuInitialized = true;
    }
  }
}

void initWifi()
{
  /**
   * Set up an access point
   * @param ssid          Pointer to the SSID (max 63 char).
   * @param passphrase    (for WPA2 min 8 char, for open use NULL)
   * @param channel       WiFi channel number, 1 - 13.
   * @param ssid_hidden   Network cloaking (0 = broadcast SSID, 1 = hide SSID)
   */
  // now start the wifi
  WiFi.mode(WIFI_AP_STA);
#if AP_MODE
  /* You can remove the password parameter if you want the AP to be open. */
  if (!WiFi.softAP(ssid, password)) {
    while(1); // Loop forever if setup didn't work
  }

  IPAddress myIP = WiFi.softAPIP();

#else
  WiFi.begin(ssid, password);

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

  IPAddress myIP = WiFi.localIP();
#endif
  Serial.println("IP: ");
  Serial.println(myIP);

  if (!udp.begin(localPort)) {
    while(1); // Loop forever if setup didn't work
  }
  Serial.println("Done");

  // Broadcast IP address.
  bndl.add("/imu/ip").add(myIP[3]);
  sendOscBundle(true);
}

void sendOscBundle(boolean broadcast) {
  if (sendOSC) {

    if (useUdp) {
      udp.beginPacket(broadcast ? broadcastIP : destIP, destPort);
      bndl.send(udp); // send the bytes to the SLIP stream
      udp.endPacket(); // mark the end of the OSC Packet
    }
    else {
      SLIPSerial.beginPacket();
      bndl.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet
    }
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
