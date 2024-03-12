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
        OSCMessage msg;
        uint16_t size = network::udp.parsePacket();
        if (size > 0) {
            if(DEBUG_MODE) {
                Serial.print("Received UDP packet of size ");
                Serial.println(size);
                Serial.print("From : ");
                Serial.print(network::udp.remoteIP());
                Serial.print(", port : ");
                Serial.println(network::udp.remotePort());
            }

            while (size--) {
                msg.fill(network::udp.read());
            }
         
            if (!msg.hasError()) {
                if (DEBUG_MODE){
                    Serial.println(F("OSC message received:"));
                    msg.send(Serial);
                    Serial.println(" ");
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
                        logger::error("OSC MESSAGE ERROR : INVALID_OSC");
                        
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

