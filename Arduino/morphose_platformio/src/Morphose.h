#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

#include "Config.h"
#include <VectorXf.h>
#include <ArduinoJson.h>

class IPAddress;


namespace morphose {

    extern int id;
    extern const char* name;
    extern int outgoingPort;
    extern bool stream;

    void initialize();

    void update();

    void setID(const int byte);

    void setName(const int id);

    void setOutgoingPort(const int id);

    void sayHello();

    void resetPosition();

    Vec2f getPosition();

    void setCurrentPosition(Vec2f newPosition);

    void sendData();

    void idleMode();

    void setIdle(bool idle);

namespace json {

    extern JsonDocument deviceData;
}


namespace navigation {
        void start();
        void startHeading(float speed, float relativeHeading = 0);
        void stepHeading();

        // Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
        float getVelocityQuality();
        void stopHeading();
        void process();
        Vec2f getVelocity();
        float getVelocityHeading(); 
        void collectData();

    }  // namespace navigation


namespace energy {
        void deepSleepLowMode(float batteryVoltage);
        void deepSleepCriticalMode(float batteryVoltage);
        void check();

    }  // namespace energy
}  // namespace morphose

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

