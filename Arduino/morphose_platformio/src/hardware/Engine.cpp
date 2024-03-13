#include "Engine.h"

#include <Dynamixel2Arduino.h>
#include <Wire.h>

#include "communications/osc.h"
#include "Utils.h"

#define MOTORS_SPEED_MAX 70
#define MOTORS_STEER_MAX 45
#define MOTORS_STEER_MIDDLE 180
#define DXL_SERIAL   Serial1

namespace motors {

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
    float engineSpeed;
    float engineSteer;
    bool enginePower;
    bool newEngineData;

    void initialize() {
        enginePower = false;
        newEngineData = false;

        // Set Port baudrate to 57600bps for DYNAMIXEL motors.
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
    }
    void update() {
        if (newEngineData) {
            newEngineData = false;
            if (enginePower) {
                dxl.torqueOn(DXL_ID_SPEED);
                dxl.torqueOn(DXL_ID_STEER);
            } else {
                dxl.torqueOff(DXL_ID_SPEED);
                dxl.torqueOff(DXL_ID_STEER);
            }

            dxl.setGoalVelocity(DXL_ID_SPEED, utils::safeRemapNorm(engineSpeed, MOTORS_SPEED_MAX), UNIT_RAW);  // +n=CCW, -n=CW
            currentSpeed = engineSpeed;
            
            dxl.setGoalPosition(DXL_ID_STEER, utils::safeRemapNorm(engineSteer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);
            currentSteer = engineSteer;
        }
    }

    void setEnginePower(bool on) {
        enginePower = on;
        newEngineData = true;
    }
    void setEngineSpeed(float speed) {
        engineSpeed = speed;
        newEngineData = true;
    }

    void setEngineSteer(float steer) {
        currentSteer = steer;
        newEngineData = true;
    }

    float getEngineSpeed() { return currentSpeed; }
    float getEngineSteer() { return currentSteer; }
    float engineIsMovingForward() { return (currentSpeed >= 0); }

    float getBatteryVoltage() {
        int voltage = dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED);
        return (voltage / 10.0f);
    }

    int getEngineSpeedTemperature() {
        return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_SPEED);
    }

    int getEngineSteerTemperature() {
        return dxl.readControlTableItem(PRESENT_TEMPERATURE, DXL_ID_STEER);
    }


    void collectData() {
        morphose::json::deviceData["speed"] = getEngineSpeed();
        morphose::json::deviceData["steer"] = getEngineSteer();
        morphose::json::deviceData["battery"] = getBatteryVoltage();
    }

}  // namespace motors
