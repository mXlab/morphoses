#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

#include <VectorXf.h>

class IPAddress;


namespace morphose {

    extern int id;
    extern char name[16];
    extern int outgoingPort;

    void initialize(IPAddress ip);

    void update();

    void setID(const int byte);

    void setName(const int id);

    void setOutgoingPort(const int id);

    void sayHello();

    Vec2f getPosition();

    void setCurrentPosition(Vec2f newPosition);

namespace navigation {
        void startNavigation();

        void startNavigationHeading(float speed, float relativeHeading = 0);

        void stepNavigationHeading();

        // Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
        float getNavigationVelocityQuality();

        void stopNavigationHeading();

        void processNavigation();

        Vec2f getVelocity();

        float getVelocityHeading();

        void sendNavigationInfo();

        void updateNavigationVelocity(bool movingForward);

    }  // namespace navigation


namespace energy {
        void deepSleepLowMode(float batteryVoltage);
        void deepSleepCriticalMode(float batteryVoltage);
        void checkEnergy();

    }  // namespace energy
}  // namespace morphose

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_MORPHOSE_H_

