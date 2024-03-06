#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSC_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSC_H_

#include <Arduino.h>
#include <OSCBundle.h>
#include <OSCMessage.h>

#include "Morphose.h"




namespace osc {
    extern OSCBundle bundle;

    bool isBroadcasting();
    void setBroadcast(bool onoff);

    /**
     * @brief OSC protocol update loop. Verify if there's an incoming message add points to the right callback
     */
    void update();

    void sendBundle();
    void sendBundle(IPAddress ip, uint16_t port);

    // void sendOscBundle(bool broadcast = false, boolean force = false, int port = morphose::outgoingPort);

     /**
     * @brief sends an OSCMessage object over UDP
     */
    void send(OSCMessage &msg);

    /**
     * @brief template to create and send an OSC message containing 1 value
     * @param addr address of the osc message to send
     * @param val value to send
     */
    template <typename T>
      void send (const char *addr,T val){
        OSCMessage msg(addr);
        msg.add(val);
        send(msg);
    }

        template <typename T>
      void reply (OSCMessage &msg,T val){
        msg.empty();
        msg.add(val);
        send(msg);
    }


    /**
     * @brief Sends a debug message to host on /debug address
     * 
     * @param _msg Message to send
     */
    void debug(const char *_msg);

    /**
     * @brief cast the specified argument of the osc message to uint8_t and returns it
     * 
     * @param msg message to get the argument from
     * @param idx index of the argement to cast
     * @return uint8_t - value of the specified argument casted to uint8_t
     */
    uint8_t castItemFromIndexToInt(OSCMessage &msg, int idx); 

    /**
     * @brief cast the specified argument of the osc message to uint16_t and returns it
     * 
     * @param msg message to get the argument from
     * @param idx index of the argement to cast
     * @return uint16_t - value of the specified argument casted to uint16_t
     */
    uint16_t castItemFromIndexToInt16(OSCMessage &msg, int idx); 

    /**
     * @brief cast the specified argument of the osc message to float and returns it
     * 
     * @param msg message to get the argument from
     * @param idx index of the argement to cast
     * @return float - value of the specified argument casted to float
     */
    float castItemFromIndexToFloat(OSCMessage &msg, int idx);

    /// Smart-converts argument from message to integer.
    bool getArgAsBool(OSCMessage& msg, int index);

    /// Smart-converts argument from message to integer.
    int32_t getArgAsInt(OSCMessage& msg, int index);

    /// Smart-converts argument from message to float.
    float getArgAsFloat(OSCMessage& msg, int index);

    /// Returns true iff argument from message is convertible to a number.
    boolean argIsNumber(OSCMessage& msg, int index);


}  // namespace osc

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_COMMUNICATIONS_OSC_H_

