// WiFi & OSC.
#include <OSCBundle.h>
#include <WiFiUdp.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

// Constants ////////////////////////////////////
#define WIFI_CONNECTION_TIMEOUT 5000

boolean sendOSC = true; // default

OSCBundle bndl;
WiFiUDP udp;

int numActiveIPs = 0;
IPAddress** destIPs;        // array of IPAddress pointers
IPAddress   broadcastIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, 255); // broadcast

// The board ID corresponds to the 4th number of its IP.
int boardID;

// The destination port is 8 followed by the boardID.
int destPort;
#if DUAL_IMU
int destPortSide;
#endif
int destPortInfo;

// A human readable name for the board.
char boardName[16];

// Function declarations ////////////////////////

/// Smart-converts argument from message to integer.
int32_t getArgAsInt(OSCMessage& msg, int index);

/// Smart-converts argument from message to float.
float getArgAsFloat(OSCMessage& msg, int index);

/// Returns true iff argument from message is convertible to a number.
boolean argIsNumber(OSCMessage& msg, int index);

// Sends currently built bundle (with optional broadcasting option).
// ** WARNING **: The beginPacket() & sendPacket() functions need to be called regularly
// otherwise the program seems to have trouble receiving data and loses some packets. It 
// is unclear why, but this seems to resolve the issue.
void sendOscBundle(boolean broadcast=false, int port=destPort, boolean force=false) {
  // loop through registered IP addresses and send same packet to each of them
  for (byte i = 0; i < numActiveIPs; i++) {
    udp.beginPacket(*destIPs[i], port);

    if (sendOSC || force) {
      bndl.send(udp); // send the bytes to the SLIP stream
    }
    udp.endPacket(); // mark the end of the OSC Packet ** keep this line** (see warning above)
  }
  bndl.empty(); // empty the bundle to free room for a new one
}

bool receiveMessage(OSCMessage& messIn) {
    // if there's data available, read a packet
  int packetSize = udp.parsePacket();
  bool messageReceived = false;

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
        messageReceived = true;

        Serial.println("Message received");
        bndl.add("/bonjour").add(boardName);
        sendOscBundle(false, true); // force send so as to always respond

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
  
  return messageReceived;
}

void initBoardInfo(int id) {
  boardID = id;
  destPort = 8000 + boardID;
  #if DUAL_IMU
  destPortSide = destPort + 1;
  #endif
  destPortInfo = destPort + 2;
  sprintf(boardName, "robot%d-%s", (boardID % 100) / 10, (boardID % 10 == 0 ? "main" : "imu"));
}

bool wifiIsConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

void addIPAddress(IPAddress ip) {
  //destIP = udp.remoteIP();
  // determine if the address we want to add is already registered...
  bool ipExists = false;
  for (int i = 0; i < numActiveIPs; i++) {
    // if the last byte matches
    if (*destIPs[i][3] === ip[3]) {
      ipExists = true;
      break;
    }
  }
  if (ipExists) return;

  // if it doesnt exist, we add it
  // first, reallocate array since we now have one more IP (size incremented here aswell)
  destIPs = (IPAddress**) realloc(destIPs, size(destIPs[0]) * (++numActiveIPs));
  // add the new ip here
  destIPs[numActiveIPs - 1] = new IPAddress(ip[0], ip[1], ip[2], ip[3]);

  // log for verification
  if (DEBUG_MODE) {
    Serial.print("added ");
    Serial.println(*destIPs[numActiveIPs - 1]);
  }
}

void initWifi()
{
  // allocate space for the IP addresses
  destIPs = new IPAddress*[1];
  numActiveIPs = 1;
  // add static IP as test
  destIPs[0] = new IPAddress(DEST_IP_0, DEST_IP_1, DEST_IP_2, DEST_IP_3);

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

  IPAddress myIP = WiFi.localIP();

  Serial.println("IP: ");
  Serial.println(myIP);

  initBoardInfo(myIP[3]);

  if (!udp.begin(LOCAL_PORT)) {
    while(1); // Loop forever if setup didn't work
  }
  Serial.println("Done");

  // Broadcast myself.
  bndl.add("/bonjour").add(boardName);
  sendOscBundle(true);
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


/// Smart-converts argument from message to float.
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
