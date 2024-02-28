#include "communications/osc.h"

#include <ArduinoLog.h>
#include <OSCBundle.h>
#include <OSCMessage.h>

#include "Network.h"
#include "Morphose.h"
#include "OSCCallbacks.h"

namespace osc {

    OSCBundle bundle{};
    bool broadcast = true;


    bool isBroadcasting(){
        return broadcast;
    }

    void setBroadcast( bool onoff){
        broadcast = onoff;
    }

    void send(const char *addr) {
        OSCMessage msg(addr);
        send(msg);
    }

    void send(OSCMessage &msg) {
        if(broadcast){
            network::udp.beginPacket(network::broadcast, network::outgoingPort);
        }else{
            network::udp.beginPacket(network::pcIP, network::outgoingPort);
        }
        msg.send(network::udp);
        network::udp.endPacket();
        msg.empty();
    }

    void debug(const char *_msg) {
        Log.infoln(_msg);  // TODO(Etienne): Remove. Only for debugging
        send<const char *>("/debug",_msg);
    }


    void sendBundle(IPAddress ip, uint16_t port) {
        //Serial.println(ip);
        //Serial.println(port);
        network::udp.beginPacket(ip,port);
        bundle.send(network::udp);
        network::udp.endPacket();
        bundle.empty();
    }

    void sendBundle() {
        if(broadcast){
            sendBundle(network::broadcast, network::outgoingPort);
        }else{
            sendBundle(network::pcIP, network::outgoingPort);
        }
    }

    // TODO(Etienne) : Verify if still needed. If not remove from code

//     void sendOscBundleToIP(const IPAddress& ip, boolean force, int port) {
//         network::udp.beginPacket(ip, port);
//         if (sendOSC || force) {
//             bundle.send(network::udp);   // send the bytes to the SLIP stream
//         }
//         network::udp.endPacket();   // mark the end of the OSC Packet ** keep this line** (see warning above)
//     }

// // Sends currently built bundle (with optional broadcasting option).
// // ** WARNING **: The beginPacket() & sendPacket() functions need to be called regularly
// // otherwise the program seems to have trouble receiving data and loses some packets. It
// // is unclear why, but this seems to resolve the issue.
// void sendOscBundle(bool broadcast, bool force, int port) {
//     if (broadcast) {
//         sendOscBundleToIP(network::broadcast, force, port);
//     } else {

//         // // loop through registered IP addresses and send same packet to each of them
//         for (byte i = 0; i < network::numActiveIPs; i++) {
//             // create temporary IP address
//             IPAddress ip(DEST_IP_0, DEST_IP_1, DEST_IP_2, network::destIPs[i]);

//             // begin packet
//             sendOscBundleToIP(ip, force, port);
//         }
//     }
//     bundle.empty();     // empty the bundle to free room for a new one
//     }


    uint8_t castItemFromIndexToInt(OSCMessage &msg, int idx) {
        uint8_t val;
        switch (msg.getType(idx)) {
            case 'i':
                val = msg.getInt(idx);
                break;
            case 'f':
                val = int(msg.getFloat(idx));
                break;
        }
        return val;
    }

    uint16_t castItemFromIndexToInt16(OSCMessage &msg, int idx) {
        uint16_t val;
        switch (msg.getType(idx)) {
            case 'i':
                val = msg.getInt(idx);
                break;
            case 'f':
                val = int(msg.getFloat(idx));
                break;
        }
        return val;
    }

    float castItemFromIndexToFloat(OSCMessage &msg, int idx) {
        float val;
        switch (msg.getType(idx)) {
            case 'i':
                val = float(msg.getInt(idx));
                break;
            case 'f':
                val = msg.getFloat(idx);
                break;
        }
        return val;
    }

/// Smart-converts argument from message to integer.
int32_t getArgAsInt(OSCMessage& msg, int index) {
  if (msg.isInt(index)) {
    return msg.getInt(index);
  } else if (msg.isBoolean(index)) {
    return (msg.getBoolean(index) ? 1 : 0);
  } else {
    double val = 0;
    if (msg.isFloat(index))       val = msg.getFloat(index);
    else if (msg.isDouble(index)) val = msg.getDouble(index);
    return round(val);
  }
}

bool getArgAsBool(OSCMessage& msg, int index) {
  return (bool)getArgAsInt(msg, index);
}

/// Smart-converts argument from message to float.
float getArgAsFloat(OSCMessage& msg, int index) {
  if (msg.isFloat(index)) {
    return msg.getFloat(index);
  } else if (msg.isDouble(index)) {
    return (float)msg.getDouble(index);
  } else if (msg.isBoolean(index)) {
    return (msg.getBoolean(index) ? 1 : 0);
  } else {
    return (float)msg.getInt(index);
}
}
/// Returns true iff argument from message is convertible to a number.
boolean argIsNumber(OSCMessage& msg, int index) {
  return (msg.isInt(index) || msg.isFloat(index) || msg.isDouble(index) || msg.isBoolean(index));
}

    void update() {
        // OSC Routine
        // Tried to wrap this in a class and in a namespace and it makes the mcu crash for unknown reasons
        // OSCMessage msg;
        OSCBundle msg;  // TODO(Etienne) :Verify if receiving bundles or messages.
        OSCMessage temp;
        uint16_t size = network::udp.parsePacket();

        if (size > 0) {

            while (size--) {
                msg.fill(network::udp.read());
            }

            if (DEBUG_MODE) {
                Serial.print("Received packet of size ");
                Serial.println(size);
                Serial.print("From : ");
                Serial.print(network::udp.remoteIP());
                Serial.print(", port : ");
                Serial.println(network::udp.remotePort());
                Serial.println(F("OSC message received:"));
                msg.send(Serial);
                Serial.println(" ");
            }

            

            if (!msg.hasError()) {
                if (DEBUG_MODE){
                    Serial.println("no errors in packet"); 
                } 
                Serial.println("Message received");
                bundle.add("/received").add(morphose::name).add(network::udp.remoteIP()[3]);
                sendBundle();
   
                msg.dispatch("/bonjour",             oscCallback::bonjour);//ok
                msg.dispatch("/broadcast",           oscCallback::broadcast);
                msg.dispatch("/get/data",            oscCallback::getData);
                msg.dispatch("/speed",               oscCallback::speed); //ok
                msg.dispatch("/steer",               oscCallback::steer); //ok
                msg.dispatch("/nav/start",           oscCallback::startNavigation);
                msg.dispatch("/nav/stop",            oscCallback::stopNavigation);
                msg.dispatch("/reboot",              oscCallback::reboot); //ok
                msg.dispatch("/stream",              oscCallback::stream);
                msg.dispatch("/power",               oscCallback::power); //ok
                msg.dispatch("/calib/begin",         oscCallback::calibrationBegin);
                msg.dispatch("/calib/end",           oscCallback::calibrationEnd);
                msg.dispatch("/calib/save",          oscCallback::saveCalibration);
                msg.dispatch("/rgb/all",             oscCallback::rgbAll); //ok
                msg.dispatch("/rgb/one",             oscCallback::rgbOne); //ok
                msg.dispatch("/rgb/region",          oscCallback::rgbRegion); //ok
                msg.dispatch("/base-color",          oscCallback::baseColor);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/alt-color",           oscCallback::altColor);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/period",              oscCallback::animationPeriod);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/noise",               oscCallback::noise);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/animation-type",      oscCallback::animationType);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/animation-region",    oscCallback::animationRegion);  // TODO(Etienne) : Verify if keeping. Not present in old sofian code
                msg.dispatch("/log",                 oscCallback::log);  // TODO(Etienne): Remove. Only for debug
                msg.dispatch("/flush",               oscCallback::readyToFlush);
                msg.dispatch("/endLog",               oscCallback::endLog);
            
            } else {    // If message contains an error;
                
                switch (msg.getError()) {
                    case BUFFER_FULL:
                        Log.errorln("OSC MESSAGE ERROR : BUFFER_FULL");
                        break;

                    case INVALID_OSC:
                        
                        Log.errorln("OSC MESSAGE ERROR : INVALID_OSC");
                        
                        break;

                    case ALLOCFAILED:
                        Log.errorln("OSC MESSAGE ERROR : ALLOCFAILED");
                        break;

                    case INDEX_OUT_OF_BOUNDS:
                        Log.errorln("OSC MESSAGE ERROR : INDEX_OUT_OF_BOUNDS");
                        break;

                    case OSC_OK:
                        Log.errorln("OSC MESSAGE ERROR : OSC_OK");

                        break;
                }
            }
        }
    }
}   // namespace osc

