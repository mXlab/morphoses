#include "Config.h"

#include <UM7.h>

#include <OSCBundle.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);

UM7 imu;

WiFiUDP udp;
OSCBundle bndl;

IPAddress destIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, DEST_IP_3); // remote IP

//int nIMU;
//int iter;

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize Wifi and UDP.
  initWifi();

//  nIMU = 0;
//  iter = 0;
}

void loop() {

  // Tick.
//  bndl.add("/tick").add(iter++).add(millis()/1000.0f).add(Serial.available()).add(nIMU);
//  sendOscBundle();
  
  // Send IMU.
  processImu();
  
}

void processImu() {
  if (Serial.available() > 0) {
//    Serial.println("Receiving data");
    if (imu.encode(Serial.read())) {
      if (sendOSC) {
//        bndl.add("/ypr/deg").add(imu.yaw/100.0).add(imu.pitch/100.0).add(imu.roll/100.0);
//        bndl.add("/ypr/deg").add(imu.yaw()).add(imu.pitch()).add(imu.roll());
        bndl.add("/quat").add(imu.q_a()).add(imu.q_b()).add(imu.q_c()).add(imu.q_d());
        sendOscBundle();
//        nIMU ++;
      }    
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

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  IPAddress myIP = WiFi.localIP();
#endif
  Serial.println("IP: ");
  Serial.println(myIP);

  if (!udp.begin(localPort)) {
    while(1); // Loop forever if setup didn't work
  }
  Serial.println("Done");
}

void sendOscBundle() {
  if (sendOSC) {

    if (useUdp) {
      udp.beginPacket(destIP, destPort);
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
