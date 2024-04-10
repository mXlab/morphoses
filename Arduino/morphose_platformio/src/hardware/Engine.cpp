#include "Engine.h"

#include <Dynamixel2Arduino.h>
#include <Wire.h>

//#include "communications/osc.h"
#include "Morphose.h"
#include "communications/asyncMqtt.h"
#include "Utils.h"

#define MOTORS_SPEED_MAX 70
#define MOTORS_STEER_MAX 45
#define MOTORS_STEER_MIDDLE 180
#define DXL_SERIAL   Serial1

namespace motors {

    // Mutex for async Dynamixel communication.
    SemaphoreHandle_t dxlMutex = NULL;

    // Dynamixel parameters ***********************************
    const uint8_t DXL_DIR_PIN = 5;  // DYNAMIXEL Shield DIR PIN
    const uint8_t DXL_ID_SPEED = 1;  // motor for rolling
    const uint8_t DXL_ID_STEER = 2;  // motor for steering left right
    const float DXL_PROTOCOL_VERSION = 2.0;

    // Dynamixel motor control object.
    Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

    // This namespace is required to use Control table item names
    using namespace ControlTableItem;

    uint32_t profile_acceleration_rolling = 30;
    uint32_t profile_velocity_rolling = 180;

    uint32_t profile_acceleration_steering = 40;
    uint32_t profile_velocity_steering = 150;
    // ********************************************************

    float currentSpeed;
    float currentSteer;
    bool enginePower;

    bool lockMutex() {
        return (xSemaphoreTake (dxlMutex, portMAX_DELAY));
    }

    void unlockMutex() {
        xSemaphoreGive(dxlMutex);  // release the mutex
    }

    void initialize() {

        // Create mutex.
        dxlMutex = xSemaphoreCreateMutex();

        enginePower = false;

        // todo : verify if baudrate is too slow
        // Set Port baudrate to 57600bps for DYNAMIXEL motors.
        if (lockMutex()) {
            dxl.begin(57600);
            // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
            dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

            // Turn off torque when configuring items in EEPROM area
            dxl.torqueOff(DXL_ID_SPEED);
            dxl.torqueOff(DXL_ID_STEER);
            dxl.setOperatingMode(DXL_ID_SPEED, OP_VELOCITY);  // rolling
            dxl.setOperatingMode(DXL_ID_STEER, OP_POSITION);  // steering left right
            dxl.torqueOn(DXL_ID_SPEED);
            dxl.torqueOn(DXL_ID_STEER);

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_SPEED, profile_acceleration_rolling);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_SPEED, profile_velocity_rolling);

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_STEER, profile_acceleration_steering);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_STEER, profile_velocity_steering);

            currentSpeed = currentSteer = 0;

            unlockMutex();
        }
    }
    void update() {
    }

    void setEnginePower(bool on) {
        if (lockMutex()) {
            enginePower = on;
            if (enginePower) {
                dxl.torqueOn(DXL_ID_SPEED);
                dxl.torqueOn(DXL_ID_STEER);
            }
            else {
                dxl.torqueOff(DXL_ID_SPEED);
                dxl.torqueOff(DXL_ID_STEER);
            }

            unlockMutex();
        }
    }

    void setEngineSpeed(float speed) {
        if (lockMutex()) {
            currentSpeed = speed;
            dxl.setGoalVelocity(DXL_ID_SPEED, utils::safeRemapNorm(currentSpeed, MOTORS_SPEED_MAX), UNIT_RAW);  // +n=CCW, -n=CW

            unlockMutex();
        }
    }
    void setEngineSteer(float steer) {
        if (lockMutex()) {
            currentSteer = steer;
            dxl.setGoalPosition(DXL_ID_STEER, utils::safeRemapNorm(currentSteer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);

            unlockMutex();
        }
    }

    //todo : should read from dxl instead
    float getEngineSpeed() { return currentSpeed; }
    float getEngineSteer() { return currentSteer; }
    float engineIsMovingForward() { return (currentSpeed >= 0); }

    float getBatteryVoltage() {
        static float voltage = 0;

        if (lockMutex()) {
            voltage = dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED) / 10.0f;
            unlockMutex();
        }

        return voltage;
    }

    int getEngineSpeedTemperature() {
        static int temperature = 0;
        if (lockMutex()) {
            temperature = dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_SPEED);
            unlockMutex();
        }
        return temperature;
    }

    int getEngineSteerTemperature() {
        static int temperature = 0;
        if (lockMutex()) {
            temperature = dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_STEER);
            unlockMutex();
        }
        return temperature;
    }

    void collectData() {
        if (lockMutex()) {

            float speed   = getEngineSpeed();
            float steer   = getEngineSteer();
            float battery = getBatteryVoltage();
            
            unlockMutex();

            morphose::json::deviceData["speed"]   = speed;
            morphose::json::deviceData["steer"]   = steer;
            morphose::json::deviceData["battery"] = battery;
        }
    }

}  // namespace motors
