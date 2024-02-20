#ifndef MORPHOSE_COMM_H
#define MORPHOSE_COMM_H

// WiFi & OSC.
#include <Arduino.h>
#include "Config.h"
#include <OSCBundle.h>
#include <OSCMessage.h>
#include <WiFiUdp.h>
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "Utils.h"


// Constants ////////////////////////////////////
//#define WIFI_CONNECTION_TIMEOUT 5000

//modev to osc
// boolean sendOSC = true; // default

//MOvED TO OSC and renamed bundle
//OSCBundle bndl;


WiFiUDP udp;

// IP address registry
#define MAX_DEST_IPS 4
byte destIPs[MAX_DEST_IPS];
int numActiveIPs = 0, lastAddedIPIndex = 0;

// broadcast IP (deprecated)
//IPAddress   broadcastIP(DEST_IP_0, DEST_IP_1, DEST_IP_2, 255); // broadcast

// The robot ID corresponds to the 2nd digit of the 4th part of the IP address.
int robotId;

// The destination port is 81 followed by the boardID.
int destPort;

// MOVED TO MORPHOSE.h
// A human readable name for the board.
//char boardName[16];

// Function declarations ////////////////////////

//MOVED TO OSC
    // /// Smart-converts argument from message to integer.
    // bool getArgAsBool(OSCMessage& msg, int index);

    // /// Smart-converts argument from message to integer.
    // int32_t getArgAsInt(OSCMessage& msg, int index);

    // /// Smart-converts argument from message to float.
    // float getArgAsFloat(OSCMessage& msg, int index);

    // /// Returns true iff argument from message is convertible to a number.
    // boolean argIsNumber(OSCMessage& msg, int index);


//TODO : Verify if already moved
// TODO : Move to osc
// void send(OSCMessage &msg) {
//   udp.beginPacket(IPAddress(192,168,0,255), LOCAL_PORT );
//   msg.send(udp);
//   udp.endPacket();
//   msg.empty();
// }



// MOVED TO OSC
// void sendOscBundleToIP(const IPAddress& ip, boolean force, int port) {

//   udp.beginPacket(ip, port);
//   if (sendOSC || force) {
//     bndl.send(udp); // send the bytes to the SLIP stream
//   }
//   udp.endPacket(); // mark the end of the OSC Packet ** keep this line** (see warning above)
// }




//MOVED TO OSC
// Sends currently built bundle (with optional broadcasting option).
// ** WARNING **: The beginPacket() & sendPacket() functions need to be called regularly
// otherwise the program seems to have trouble receiving data and loses some packets. It 
// is unclear why, but this seems to resolve the issue.
// void sendOscBundle(boolean broadcast=false, boolean force=false, int port=destPort) {
//   if (broadcast) {
//     sendOscBundleToIP(broadcastIP, force, port);
//   } 
  
//   else {
//     // loop through registered IP addresses and send same packet to each of them
//     for (byte i = 0; i < numActiveIPs; i++) {
    
//       // create temporary IP address
//       IPAddress ip(DEST_IP_0, DEST_IP_1, DEST_IP_2, destIPs[i]);

//       // begin packet
//       sendOscBundleToIP(ip, force, port);
//     }
//   }
//   bndl.empty(); // empty the bundle to free room for a new one
// }


bool receiveMessage(OSCMessage& messIn, IPAddress* returnRemoteIP=0) {
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
        
        //commented because it prevented to add another ip adress to ip list. Sending a message before processing 
        //received messaged replace the remoteIP for the default ip .100
        //bndl.add("/received").add(boardName).add(remote[3]);
        //sendOscBundle(false, true); // force send so as to always respond

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

    if (returnRemoteIP)
      *returnRemoteIP = remote;
      
  } //if (packetSize)
  
  return messageReceived;
}


// Moved to morphose.h
// void initBoardInfo(int id) {
//   robotId = (id % 100) / 10;
//   destPort = 8000 + id;
//   sprintf(boardName, "robot%d", robotId); // eg. "robot1"
// }

// bool wifiIsConnected() {
//   return (WiFi.status() == WL_CONNECTED);
// }

void addDestinationIPAddress(byte ip3) {
  // Determine if the address we want to add is already registered.
  char buff[32];
  sprintf(buff,"ip3: %d",ip3);
  utils::debug(buff);
  bool ipExists = false;
  for (int i = 0; i < numActiveIPs; i++) {
    // if the last byte matches
    if (destIPs[i] == ip3) {
      ipExists = true;
      break;
    }
  }
  
  if (!ipExists) {
  
    // if it doesn't exist, we add it
    numActiveIPs = min(numActiveIPs+1, MAX_DEST_IPS); // cap at max length
  
    // if we overflow, we go back to position 1
    // position 0 is reserved for the ML system's IP address
    lastAddedIPIndex = (++lastAddedIPIndex - 1) % MAX_DEST_IPS + 1;    // loop between 1 and MAX_DEST_IPS
  
    destIPs[lastAddedIPIndex] = ip3;
  
    // log for verification
    if (DEBUG_MODE) {
      Serial.print("NEW DEST. IP ADDED: ");
      Serial.println(ip3);
    }
  }
}







// void initWifi()
// {
  
//   // add static IP as test
//   destIPs[0] = DEST_IP_3;
//   numActiveIPs = 1;

//   // now start the wifi
//   WiFi.mode(WIFI_STA); 
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//   // Wait for connection to complete.
  
//   unsigned long startMillis = millis();
//   while (WiFi.status() != WL_CONNECTED && 
//          millis() - startMillis < WIFI_CONNECTION_TIMEOUT) {
//     Serial.print(".");
//     utils::blinkIndicatorLed(500);
//   }

//   // If still not connected, restart the board.
//   if (WiFi.status() != WL_CONNECTED) {
//     utils::blinkIndicatorLed(100, 0.7, 20);
//     ESP.restart();
//   }

//   IPAddress myIP = WiFi.localIP();

//   Serial.println("IP: ");
//   Serial.println(myIP);


//   //TODO : Move to morphose
//   initBoardInfo(myIP[3]);

//   if (!udp.begin(LOCAL_PORT)) {
//     while(1); // Loop forever if setup didn't work
//   }
//   Serial.println("Done");

//   //temp wifi debug
//   char buff[64];
//   sprintf(buff,"Wifi rssi : %d" ,WiFi.RSSI() );
//   utils::debug(buff);

//   // on disconnect wifi event
//   WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
//         Serial.print("WiFi lost connection. Reason: ");
//         Serial.println(info.wifi_sta_disconnected.reason);
//     }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);



//   //TODO : move to morphose maybe
//   // Broadcast myself.
//   bndl.add("/bonjour").add(boardName);
//   sendOscBundle(true);
// }

//MOVED TO OSC
    // bool getArgAsBool(OSCMessage& msg, int index) {
    //   return (bool)getArgAsInt(msg, index);
    // }

    // /// Smart-converts argument from message to integer.
    // int32_t getArgAsInt(OSCMessage& msg, int index) {
    //   if (msg.isInt(index))
    //     return msg.getInt(index);
    //   else if (msg.isBoolean(index))
    //     return (msg.getBoolean(index) ? 1 : 0);
    //   else {
    //     double val = 0;
    //     if (msg.isFloat(index))       val = msg.getFloat(index);
    //     else if (msg.isDouble(index)) val = msg.getDouble(index);
    //     return round(val);
    //   }
    // }


    // /// Smart-converts argument from message to float.
    // float getArgAsFloat(OSCMessage& msg, int index) {
    //   if (msg.isFloat(index))
    //     return msg.getFloat(index);
    //   else if (msg.isDouble(index))
    //     return (float)msg.getDouble(index);
    //   else if (msg.isBoolean(index))
    //     return (msg.getBoolean(index) ? 1 : 0);
    //   else
    //     return (float)msg.getInt(index);
    // }

    // /// Returns true iff argument from message is convertible to a number.
    // boolean argIsNumber(OSCMessage& msg, int index) {
    //   return (msg.isInt(index) || msg.isFloat(index) || msg.isDouble(index) || msg.isBoolean(index));
    // }


#endif