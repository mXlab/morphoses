#ifndef OSC_H
#define OSC_H
#include <Arduino.h>
#include <OSCBundle.h>

#define USE_BUNDLE true;

/**
 * TODO:
 * - Change oscmessages for bundle in send()
 * - examine if timetag is needed to properly sequence actions
 * 
 */


class OSCMessage; // forward declaration



namespace osc
{   
    extern OSCBundle bundle;

    /**
     * @brief OSC protocol update loop. Verify if there's an incoming message add points to the right callback
     */
    void update();


    /**
     * @brief create and send an OSC message containing a bool 
     * @param addr address of the osc message to send
     * @param val value to send
     */
    void send(const char *addr, bool val);

    void send(const char *addr, char* val);

    /**
     * @brief create and send an OSC message containing an uint8_t 
     * @param addr address of the osc message to send
     * @param val value to send
     */
    void send(const char *addr, uint8_t val);

    /**
     * @brief create and send an OSC message containing an uint16_t 
     * @param addr address of the osc message to send
     * @param val value to send
     */
    void send(const char *addr, uint16_t val);

    /**
     * @brief create and send an OSC message containing three uint16_t value 
     * @param addr address of the osc message to send
     * @param val value to send
     */
    void send(const char *addr, uint16_t x,uint16_t y, uint16_t z );

    /**
     * @brief create and send an OSC message containing a float value 
     * @param addr address of the osc message to send
     * @param val value to send
     */
    void send(const char *addr, float val);

    void sendBundle();
    /**
     * @brief create and send osc message containing motor information
     * 
     * @param addr osc message address (use information name. must start with "/")
     * @param id id of the motor's information
     * @param val value of the information to send
     */
    void send(const char *addr, uint8_t id, uint8_t val);

    /**
     * @brief sends an OSCMessage object over UDP
     */
    void send(OSCMessage &msg);


    /**
     * @brief Sends wifi signal strength to host.
     */
    void sendSignalStrength(OSCMessage &msg);
    
    /**
     * @brief Write the ID received to EEPROM and restart MisBkit
     */
    void setMisBKitId(OSCMessage &msg);

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

    
    /**
     * @brief Sends a confirmation to host that the action asked is over
     */
    void confirm(OSCMessage &msg);
} // namespace osc

#endif
