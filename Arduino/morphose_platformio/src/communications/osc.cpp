#include "osc.h"
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <ArduinoLog.h>
#include "Network.h"
#include "Morphose.h"
#include "OSCCallbacks.h"

namespace osc {

    OSCBundle bundle{};

    void confirm(OSCMessage &msg) {
        msg.empty();
        msg.add(true);
        osc::send(msg);
    }

    void send(const char *addr, const uint8_t id, const uint8_t val) {
        
        OSCMessage msg(addr);

        msg.add(id);
        msg.add(val);


        send(msg);
    }

    void send(const char *addr, const bool val) {
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }


    void send(const char *addr, const uint8_t val) {
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }

    void send(const char *addr, const uint16_t val) {
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }

    void send(const char *addr, const uint16_t x, const uint16_t y, const uint16_t z) {
        OSCMessage msg(addr);
        msg.add(x);
        msg.add(y);
        msg.add(z);
        send(msg);
    }

    void send(const char *addr, const float val) {
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }

    void send(const char *addr, char* val) {
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }

    void send(const char *addr) {
        OSCMessage msg(addr);
        send(msg);
    }

    void send(OSCMessage &msg) {
        network::udp.beginPacket(network::pcIP, morphose::outgoingPort);
        msg.send(network::udp);
        network::udp.endPacket();
        msg.empty();
    }

    void sendBundle(){
        network::udp.beginPacket(network::pcIP, morphose::outgoingPort);
        bundle.send(network::udp);
        network::udp.endPacket();
        bundle.empty();
    }

    void debug(const char *_msg) {
        OSCMessage msg("/debug");
        msg.add(_msg);
        send(msg);
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

    void update() {
        // OSC Routine
        // Tried to wrap this in a class and in a namespace and it makes the mcu crash for unknown reasons
        //OSCMessage msg;
        OSCBundle bundle;
        uint16_t size = network::udp.parsePacket();
        
        if (size > 0) {
            while (size--) {
                bundle.fill(network::udp.read());
            }

            if (!bundle.hasError()) { 
                // if message has no errors
                //TODO : set dispatch here
                bundle.dispatch("/bonjour",oscCallback::bonjour);
            }
            else // If message contains an error
            {
                switch (bundle.getError()) {
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
    
} // namespace osc
