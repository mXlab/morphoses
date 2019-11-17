#include "Config.h"

#include <SparkFun_BNO080_Arduino_Library.h>

#include <OSCBundle.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);

BNO080 imu;
WiFiUDP udp;
OSCBundle bndl;

IPAddress destIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, DEST_IP_3); // remote IP

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize Wifi and UDP.
  initWifi();

  Wire.begin();
  if (imu.begin() == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  imu.enableRotationVector(50); //Send data update every 50ms
}

void loop() {
  // Send IMU.
  processImu();
  
}

void processImu() {
  if (imu.dataAvailable() == true)
  {
//    float quatI = imu.getQuatI();
//    float quatJ = imu.getQuatJ();
//    float quatK = imu.getQuatK();
//    float quatReal = imu.getQuatReal();
//    float quatRadianAccuracy = imu.getQuatRadianAccuracy();

    if (sendOSC) {
      bndl.add("/quat").add(imu.getQuatI()).add(imu.getQuatJ()).add(imu.getQuatK()).add(imu.getQuatReal());
      sendOscBundle();
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
