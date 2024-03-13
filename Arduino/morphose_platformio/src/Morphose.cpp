#include "Morphose.h"

#include <ArduinoLog.h>
#include <Chrono.h>
#include <PlaquetteLib.h>
#include <VectorXf.h>
#include <WiFi.h>

// #include "Globals.h"
#include "hardware/IMU.h"
#include "hardware/Engine.h"
#include "communications/Network.h"
#include "communications/osc.h"
#include "communications/asyncMqtt.h"

#include "Utils.h"
#include "Logger.h"


#define AVG_POSITION_TIME_WINDOW 0.2f

namespace morphose {

    int id = ROBOT_ID;

    #if ROBOT_ID == 1
    int outgoingPort = 8110;
    char* name = "robot1";
    #elif ROBOT_ID == 2
    int outgoingPort = 8120;
    char* name = "robot2";
    #elif ROBOT_ID == 3
    int outgoingPort = 8130;
    char* name = "robot3";
    #elif ROBOT_ID == 4
    int outgoingPort = 8140;
    char* name = "robot4";
    #endif

    bool stream = true;
    Chrono sendRate{true};

    Vec2f currPosition;
    pq::Smoother avgPositionX(AVG_POSITION_TIME_WINDOW);
    pq::Smoother avgPositionY(AVG_POSITION_TIME_WINDOW);
    Vec2f avgPosition;

namespace json {
    JSONVar deviceData;
}


    void initialize() {

        Log.infoln("Robot id is set to : %d", id);
        Log.infoln("Robot name is : %s", name);
         network::outgoingPort = outgoingPort; // sets port in network file for desired robot port.
        Log.infoln("Robot streaming port is : %d", network::outgoingPort);
        Log.warningln("Morphose successfully initialized");
    }

    // void setID(const int byte) {
    //     id = ROBOT_ID
    //     Log.infoln("Robot id is set to : %d", id);
    // }

    // void setName(const int id) {
    //     sprintf(name, "robot%d", id);
    //     Log.infoln("Robot name is : %s", name);
    // }

    // void setOutgoingPort(const int id) {
    //     outgoingPort = 8100 + (id*10);
    //     network::outgoingPort = outgoingPort; // sets port in network file for desired robot port.
    //     Log.infoln("Robot streaming port is : %d", network::outgoingPort);
    // }

    void sayHello() {
        bool lastState = osc::isBroadcasting();
        osc::setBroadcast(true);
        osc::bundle.add("/bonjour").add(name);
        osc::sendBundle();
        osc::setBroadcast(lastState); 
    }

    void resetPosition(){
        currPosition.set(0, 0);
        avgPosition.set(0, 0);
        avgPositionX.reset();
        avgPositionY.reset();
    }

    Vec2f getPosition() {
    return avgPosition;
    }

    void setCurrentPosition(Vec2f newPosition) {
        currPosition.set(newPosition);
        Log.infoln("New position - x : %F y: %F", newPosition.x, newPosition.y);
    }

    void updateLocation() {
    // Update average positioning.
    // osc::debug("Updating position");
    avgPositionX.put(currPosition.x);
    avgPositionY.put(currPosition.y);
    avgPosition.set(avgPositionX.get(), avgPositionY.get());
    }

    void update() {
        updateLocation();

        if(sendRate.hasPassed(STREAM_INTERVAL, true)){
            imus::process();
            morphose::navigation::process();

            if(stream){
                sendData();
                // Send OSC bundle.
                // osc::sendBundle();
                // publish JSON data
            }
            energy::check();  // Energy checkpoint to prevent damage when low
        }
    }

    void sendData() {
        //osc::debug("Sending data");
        imus::process();
        morphose::navigation::process();
        morphose::navigation::sendInfo();
        motors::sendEngineInfo();
        // not ideal? should use static buffer
        auto jsonString = JSON.stringify(json::deviceData);
        mqtt::client.publish(name, 0, true, jsonString);
    }

namespace navigation {
        #define STEER_MAX 0.5f
        #define HEADING_FRONT_TOLERANCE 30
        const int HEADING_FRONT_MAX = 90 + HEADING_FRONT_TOLERANCE;
        const float STEER_HEADING_FRONT_MAX = sin(radians(HEADING_FRONT_MAX));

        #define NAVIGATION_ERROR_THRESHOLD 0.2f  // threshold above which measurement is considered valid
        #define MAX_NAVIGATION_ERROR 45
        #define MIN_RELIABLE_NAVIGATION_DISTANCE 0.15
        #define MAX_RELIABLE_NAVIGATION_DISTANCE 0.4

        const Vec2f REFERENCE_ORIENTATION(1, 0);

        float cumulativeNavigationError;  // cumulative navigation errors
        unsigned int nNavigationSteps;
        bool navigationMode;

        float targetHeading;
        float targetSpeed;

        Vec2f startingPosition;
        Vec2f velocity;

        float velocityHeading;
        Chrono velocityTimer;

        void start() {
        // Start navigation mode.
        navigationMode = true;

        // Save starting position.
        startingPosition.set(morphose::getPosition());
        velocityTimer.start();

        // Reset errors.
        cumulativeNavigationError = 0;
        nNavigationSteps = 0;
        velocity.set(0, 0);
        }

        void startHeading(float speed, float relativeHeading) {
        // Get current heading.
        float currentHeading = imus::getHeading();

        // Set target heading.
        targetHeading = - utils::wrapAngle180(currentHeading + relativeHeading);

        // Set target speed.
        targetSpeed = max(speed, 0.0f);

        start();
        }


        void stepHeading() {
        // Check correction. Positive: too much to the left; negative: too much to the right.
        float relativeHeading = utils::wrapAngle180(targetHeading + imus::getHeading());
        float absoluteRelativeHeading = abs(relativeHeading);

        // Compute speed.
        // We use a tolerance in order to force the robot to favor moving forward when it is at almost 90 degrees to avoid situations
        // where it just moves forward and backwards forever. It will move forward  at +- (90 + HEADING_FRONT_TOLERANCE).
        float speed = targetSpeed * (absoluteRelativeHeading < HEADING_FRONT_MAX ? +1 : -1);

        // Compute navigation error.
        float navigationError = (speed > 0 ? absoluteRelativeHeading : 180 - absoluteRelativeHeading);

        // If we are too much away from our direction, reset.
        if (navigationError >= MAX_NAVIGATION_ERROR) {
            start();
        } else {
            cumulativeNavigationError += navigationError;
            nNavigationSteps++;
        }

        // Base steering in [-1, 1] according to relative heading.
        float baseSteer = sin(radians(relativeHeading));

        // Decompose base steer in sign and absolute value.
        float steerSign = copysignf(1, baseSteer);
        float steerValue = abs(baseSteer);

        // Recompute steer in [-1, 1] based on clamped value.
        float steer = steerSign * STEER_MAX * constrain(steerValue/STEER_HEADING_FRONT_MAX, 0, 1);

        // Set speed and steer.
        motors::setEngineSpeed(speed);
        motors::setEngineSteer(steer);
        }

        // Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
        float getVelocityQuality() {
        // First part of the error depends on distance moved: longer distances are more reliable.
        float absoluteMovement = velocity.length();  // absolute distance covered
        float movementQuality = pq::mapFloat(absoluteMovement, MIN_RELIABLE_NAVIGATION_DISTANCE, MAX_RELIABLE_NAVIGATION_DISTANCE, 0, 1);
        movementQuality = constrain(movementQuality, 0, 1);

        // Second part of the error depends on average deviation from target during navigation.
        float navigationQuality = (nNavigationSteps > 0 ? pq::mapFloat(cumulativeNavigationError / nNavigationSteps, 0, MAX_NAVIGATION_ERROR, 1, 0) : 0);

        // Return the average of both parts.
        return (movementQuality + navigationQuality) / 2.0f;
        }

        void stopHeading() {
        // Update navigation velocity.
        velocity = (morphose::getPosition() - startingPosition);
        velocityHeading = REFERENCE_ORIENTATION.angle(velocity);
        if (!motors::engineIsMovingForward()) velocityHeading = utils::wrapAngle180(velocityHeading + 180);

        // Align IMU offset to velocity heading.
        if (getVelocityQuality() >= NAVIGATION_ERROR_THRESHOLD)
            imus::tare(velocityHeading);

        // Reset engine.
        motors::setEngineSpeed(0);
        motors::setEngineSteer(0);

        // Exit navigation mode.
        navigationMode = false;
        targetHeading = 0;
        targetSpeed = 0;
        }

        void process() {
            if (navigationMode) {
                stepHeading();
            }
        }


        Vec2f getVelocity() {
        return velocity;
        }

        float getVelocityHeading() {
        return velocityHeading;
        }

        void sendInfo() {
            json::deviceData["/heading"] = imus::getHeading();
            json::deviceData["/velocity/x"] = getVelocity().x;
            json::deviceData["/velocity/y"] = getVelocity().y;
            json::deviceData["/heading-quality"] = getVelocityQuality();
        // osc::bundle.add("/heading").add(imus::getHeading());
        // osc::bundle.add("/velocity").add(getVelocity().x).add(getVelocity().y);
        // osc::bundle.add("/heading-quality").add(getVelocityQuality());
        }

    }  // namespace navigation

namespace energy {
        #define ENERGY_VOLTAGE_LOW_WAKEUP_TIME 10
        #define ENERGY_VOLTAGE_OK_MODE       0
        #define ENERGY_VOLTAGE_LOW_MODE      1
        #define ENERGY_VOLTAGE_CRITICAL_MODE 2

        // RTC_DATA_ATTR uint8_t energyVoltageMode   = ENERGY_VOLTAGE_OK_MODE;
        // RTC_DATA_ATTR float   savedBatteryVoltage = -1;

        void deepSleepLowMode(float batteryVoltage) {
            // osc::bundle.add("/error").add("battery-low").add(batteryVoltage);
            // osc::sendBundle();
            osc::debug("Battery low");
            delay(1000);    // TODO(Etienne): Verify with sofian why delay here
            // Wakeup every 10 seconds.
            esp_sleep_enable_timer_wakeup(ENERGY_VOLTAGE_LOW_WAKEUP_TIME * 1000000UL);

            // Go to sleep.
            esp_deep_sleep_start();
        }

        void deepSleepCriticalMode(float batteryVoltage) {
            // osc::bundle.add("/error").add("battery-critical").add(batteryVoltage);
            // osc::sendBundle();
            osc::debug("Battery critical");

            delay(1000);    // TODO(Etienne): Verify with sofian why delay here

            // Go to sleep forever.
            esp_deep_sleep_start();
        }

        void check() {
            #if defined(MORPHOSE_DEBUG)
                //Serial.println("Checking energy");
            #endif
            // Read battery voltage.
            // osc::debug("Checking voltage");
            float batteryVoltage = motors::getBatteryVoltage();
            
            // Low voltage: Launch safety procedure.
            if (batteryVoltage < ENERGY_VOLTAGE_LOW) {
                // Put IMUs to sleep to protect them.
                osc::debug("Voltage low");
                //logger::error("Voltage low");
                imus::sleep();

                // Power engine off.
                motors::setEnginePower(false);

                // If energy level is critical, just shut down the ESP.
                if (batteryVoltage < ENERGY_VOLTAGE_CRITICAL){
                    osc::debug("Voltage Critical");
                    //logger::error("Voltage Critical");

                deepSleepCriticalMode(batteryVoltage);

                }

                // Otherwise, sleep but wake up to show that something is wrong.
                else{
                deepSleepLowMode(batteryVoltage);
                }
            }
        }
    }  // namespace energy
}  // namespace morphose
