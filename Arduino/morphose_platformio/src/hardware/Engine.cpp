#include "Engine.h"

#include <Dynamixel2Arduino.h>
#include <Wire.h>


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

    const uint32_t PROFILE_ACCELERATION_SPEED = 30;
    const uint32_t PROFILE_VELOCITY_SPEED = 180;

    const uint32_t PROFILE_ACCELERATION_STEER = 40;
    const uint32_t PROFILE_VELOCITY_STEER = 150;
    // ********************************************************

    float currentSpeed;
    float currentSteer;

    bool speedTemperatureCritical;
    bool steerTemperatureCritical;

    bool engineSpeedPower;
    bool engineSteerPower;

    bool lockMutex() {
        return (xSemaphoreTake (dxlMutex, portMAX_DELAY));
    }

    void unlockMutex() {
        xSemaphoreGive(dxlMutex);  // release the mutex
    }

    void initialize() {

        // Create mutex.
        dxlMutex = xSemaphoreCreateMutex();

        engineSpeedPower = engineSteerPower = false;
        speedTemperatureCritical = steerTemperatureCritical = false;

        // todo : verify if baudrate is too slow
        // Set Port baudrate to 57600bps for DYNAMIXEL motors.
        if (lockMutex()) {
            dxl.begin(57600);
            // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
            dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
            
            //check for motor at old baudrate and change it for new if found
            if(dxl.ping(DXL_ID_SPEED)){
                Serial.println("found speed motor with baud 57600");
                dxl.torqueOff(DXL_ID_SPEED);
                dxl.setBaudrate(DXL_ID_SPEED, 1000000);
            }

            if(dxl.ping(DXL_ID_STEER)){
                Serial.println("found steer motor with baud 57600");
                dxl.torqueOff(DXL_ID_STEER);
                dxl.setBaudrate(DXL_ID_STEER, 1000000);
            }

            dxl.begin(1000000);
            //check if find motor at new baudrate
            if(dxl.ping(DXL_ID_SPEED)){
                Serial.println("found speed motor with baud 1000000");
            }
            if(dxl.ping(DXL_ID_STEER)){
                Serial.println("found steer motor with baud 1000000");
            }

            // Turn off torque when configuring items in EEPROM area
            dxl.torqueOff(DXL_ID_SPEED);
            dxl.torqueOff(DXL_ID_STEER);
            dxl.setOperatingMode(DXL_ID_SPEED, OP_VELOCITY);  // rolling
            dxl.setOperatingMode(DXL_ID_STEER, OP_POSITION);  // steering left right
            dxl.torqueOn(DXL_ID_SPEED);
            dxl.torqueOn(DXL_ID_STEER);

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_SPEED, PROFILE_ACCELERATION_SPEED);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_SPEED, PROFILE_VELOCITY_SPEED);

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_STEER, PROFILE_ACCELERATION_STEER);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_STEER, PROFILE_VELOCITY_STEER);

            currentSpeed = currentSteer = 0;

            unlockMutex();
        }
    }


    void checkTemperature() {
        static bool engineSpeedPowerSave = false;
        static bool engineSteerPowerSave = false;

        int speedTemperature = getEngineSpeedTemperature();
        int steerTemperature = getEngineSteerTemperature();
        
        

        // High temperature: Launch safety procedure.

        // Critical speed motor temperature: disable motor.
        if (speedTemperature >= MOTOR_TEMPERATURE_CRITICAL) {
            speedTemperatureCritical = true;
            engineSpeedPowerSave = engineSpeedPower; // Keep trace of current power state.
            setEngineSpeedPower(false);
        }
        // Speed motor has cooled down: re-enable motor.
        else if (speedTemperatureCritical && speedTemperature <= MOTOR_TEMPERATURE_COOLDOWN) {
            speedTemperatureCritical = false;
            setEngineSpeedPower(engineSpeedPowerSave); // Reset engine power state.
        }

        // Critical steering motor temperature: disable motor.
        if (steerTemperature >= MOTOR_TEMPERATURE_CRITICAL) {
            steerTemperatureCritical = true;
            engineSteerPowerSave = engineSteerPower; // Keep trace of current power state.
            setEngineSteerPower(false);
        }
        // Steering motor has cooled down: re-enable motor.
        else if (steerTemperatureCritical && steerTemperature <= MOTOR_TEMPERATURE_COOLDOWN) {
            steerTemperatureCritical = false;
            setEngineSteerPower(engineSteerPowerSave);  // Reset engine power state.
        }

        // Sending debug messages.
        if (speedTemperatureCritical) {
            char buffer[64];
            sprintf(buffer,"Speed motor disabled due to critical temperature: %d.", speedTemperature);
            mqtt::debug(buffer);
        }
        if (steerTemperatureCritical) {
            char buffer[64];
            sprintf(buffer,"Steer motor disabled due to critical temperature: %d.", steerTemperature);
            mqtt::debug(buffer);
        }


        char buffer[64];
        sprintf(buffer,"%d,%d,%d,%d", speedTemperature,speedTemperatureCritical, steerTemperature,steerTemperatureCritical);
        mqtt::sendTemperature(buffer);
    }

    void setEnginePower(bool on) {
        setEngineSpeedPower(on);
        setEngineSteerPower(on);
    }

    void setEngineSpeedPower(bool on) {
        if (lockMutex()) {
            engineSpeedPower = on;

            // Turn off.
            if (!on)
                dxl.torqueOff(DXL_ID_SPEED);
            
            // If temperature is critical, do not turn on.
            else if (!speedTemperatureCritical)
                dxl.torqueOn(DXL_ID_SPEED);

            unlockMutex();
        }
    }

    void setEngineSteerPower(bool on) {
        if (lockMutex()) {
            engineSteerPower = on;

            // Turn off.
            if (!on)
                dxl.torqueOff(DXL_ID_STEER);
            
            // If temperature is critical, do not turn on.
            else if (!steerTemperatureCritical)
                dxl.torqueOn(DXL_ID_STEER);

            unlockMutex();
        }
    }

    void setEngineSpeed(float speed) {
        if (speedTemperatureCritical)
            return;
        
        if (lockMutex()) {
            currentSpeed = speed;
            dxl.setGoalVelocity(DXL_ID_SPEED, utils::safeRemapNorm(currentSpeed, MOTORS_SPEED_MAX), UNIT_RAW);  // +n=CCW, -n=CW
            unlockMutex();
        }
    }
    void setEngineSteer(float steer) {
        if (steerTemperatureCritical)
            return;
        
        if (lockMutex()) {
            currentSteer = steer;
            dxl.setGoalPosition(DXL_ID_STEER, utils::safeRemapNorm(currentSteer, MOTORS_STEER_MAX, MOTORS_STEER_MIDDLE), UNIT_DEGREE);
            unlockMutex();
        }
    }

    //todo : should read from dxl instead?
    float getEngineSpeed() { return currentSpeed; }
    float getEngineSteer() { return currentSteer; }
    float engineIsMovingForward() { return (currentSpeed >= 0); }

    float getBatteryVoltage() {
        static float voltage = 0;
        if (lockMutex()) {
            float v = dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, DXL_ID_SPEED,100U) / 10.0f; 
            if(v == 0){
                dxlPacketErrorToString(dxl.getLastStatusPacketError());
                dxlLibErrorToString(dxl.getLastLibErrCode());
            }else{
                voltage = v;
            }
        }
        unlockMutex();
        return voltage;
    }


// todo : these functions could be collapsed into one if we pass motor id as a parameter
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


            float speed   = getEngineSpeed();
            float steer   = getEngineSteer();
            float battery = getBatteryVoltage();

            morphose::json::deviceData["speed"]   = speed;
            morphose::json::deviceData["steer"]   = steer;
            morphose::json::deviceData["battery"] = battery;

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
