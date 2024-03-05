#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

#include <VectorXf.h>

class IPAddress;


namespace morphose {

    extern int id;
    extern char name[16];
    extern int outgoingPort;
    extern bool stream;

    void initialize(IPAddress ip);

    void update();

    void setID(const int byte);

    void setName(const int id);

    void setOutgoingPort(const int id);

    void sayHello();

    void resetPosition();

    Vec2f getPosition();

    void setCurrentPosition(Vec2f newPosition);

    void sendData();

namespace navigation {
        void start();

        void startHeading(float speed, float relativeHeading = 0);

        void stepHeading();

        // Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
        float getVelocityQuality();

        void stopHeading();

        void process();

        Vec2f getVelocity();

        float getVelocityHeading(); //TODO(Etienne) :Never used verify if keep

        void sendInfo();

    }  // namespace navigation


namespace energy {
        void deepSleepLowMode(float batteryVoltage);
        void deepSleepCriticalMode(float batteryVoltage);
        void check();

    }  // namespace energy
}  // namespace morphose

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

