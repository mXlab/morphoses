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

    bool updateEnginePower = false;
    bool enginePower;

    bool updateEngineSpeed = false;
    bool updateEngineSteer = false;

    bool newEngineData;

    void initialize() {
        //why?
        enginePower = false;
        newEngineData = false;

        // todo : verify if baudrate is too slow
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
        
        // TODO : verify if only update once.
        if (updateEnginePower) {
            mqtt::debug("Update engine power");
            updateEnginePower = false;
            if (enginePower) {
                dxl.torqueOn(DXL_ID_SPEED);
                dxl.torqueOn(DXL_ID_STEER);
            } else {
                dxl.torqueOff(DXL_ID_SPEED);
                dxl.torqueOff(DXL_ID_STEER);
            }
        }

        // motor torque is off. no need to update anything
        if(!enginePower){ 
            //Serial.println("motors off. leaving function");
            return;
        }else{
            //Serial.println("motors on. continuing");
            if(updateEngineSteer){
                updateEngineSteer = false;
                dxl.setGoalPosition(DXL_ID_STEER, utils::safeRemapNorm(engineSteer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);
                currentSteer = engineSteer;
            }

            if(updateEngineSpeed){
                updateEngineSpeed = false;
                dxl.setGoalVelocity(DXL_ID_SPEED, utils::safeRemapNorm(engineSpeed, MOTORS_SPEED_MAX), UNIT_RAW);
                currentSpeed = engineSpeed;
            }

        }


    }

    void setEnginePower(bool on) {
        enginePower = on;
        updateEnginePower = true;
    }
    void setEngineSpeed(float speed) {
        engineSpeed = speed;
        updateEngineSpeed = true;
    }
    void setEngineSteer(float steer) {
        engineSteer = steer;
        updateEngineSteer = true;
    }

    //todo : should read from dxl instead
    float getEngineSpeed() { return currentSpeed; }
    float getEngineSteer() { return currentSteer; }
    float engineIsMovingForward() { return (currentSpeed >= 0); }

    float getBatteryVoltage() {
        int voltage = dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED, 100U);
        
        if(voltage == 0){
            dxlPacketErrorToString(dxl.getLastStatusPacketError());
            dxlLibErrorToString(dxl.getLastLibErrCode());

        }
        // todo : maybe add fail safe if return 0
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

    void dxlPacketErrorToString(int  error){
        switch (error)
        {
        case 0:
            mqtt::debug("DXL PACKET ERROR: Result Fail");
            break;
        case 1:
            mqtt::debug("DXL PACKET ERROR: Instruction Error");
            break;
        case 2:
            mqtt::debug("DXL PACKET PERROR: CRC Error");
            break;
        case 3:
            mqtt::debug("DXL PACKET ERROR: Data Range Error");
            break;
        case 4:
            mqtt::debug("DXL PACKET ERROR: Data Length Error");
            break;
        case 5:
            mqtt::debug("DXL PACKET ERROR: Data Limit Error");
            break;
        case 6:
            mqtt::debug("DXL PACKET ERROR: Access Error");
            break;
        }
    }
    void dxlLibErrorToString(DXLLibErrorCode_t  error){
        switch (error)
        {
        case 0:
            mqtt::debug("DXL ERROR: DXL_LIB_OK");
            break;
        case 1:
            mqtt::debug("DXL ERROR: DXL_LIB_PROCEEDING");
            break;
        case 2:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_NOT_SUPPORTED");
            break;
        case 3:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_TIMEOUT");
            break;
        case 4:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_INVAILD_ID");
            break;
        case 5:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_NOT_SUPPORT_BROADCAST");
            break;
        case 6:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_NULLPTR");
            break;
        case 7:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_LENGTH");
            break;
        case 8:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_INVAILD_ADDR");
            break;
        case 9:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_ADDR_LENGTH");
            break;
        case 10:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_BUFFER_OVERFLOW");
            break;
        case 11:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_PORT_NOT_OPEN");
            break;
        case 12:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_WRONG_PACKET");
            break;
        case 13:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_CHECK_SUM");
            break;
        case 14:
            mqtt::debug("DXL ERROR: DXL_LIB_ERROR_CRC");
            break;
        
        default:
            break;
        }
    }

}  // namespace motors
