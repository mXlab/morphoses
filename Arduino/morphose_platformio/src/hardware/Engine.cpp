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

    uint32_t profile_acceleration_rolling = 30;
    uint32_t profile_velocity_rolling = 180;

    uint32_t profile_acceleration_steering = 40;
    uint32_t profile_velocity_steering = 150;
    // ********************************************************

    float currentSpeed;
    float currentSteer;
    bool enginePower; // todo : remove?

    bool lockMutex() {
        return (xSemaphoreTake (dxlMutex, portMAX_DELAY));
    }

    void unlockMutex() {
        xSemaphoreGive(dxlMutex);  // release the mutex
    }

    void initialize() {

        // Create mutex.
        dxlMutex = xSemaphoreCreateMutex();

        enginePower = false; // todo: remove?

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

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_SPEED, profile_acceleration_rolling);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_SPEED, profile_velocity_rolling);

            dxl.writeControlTableItem(PROFILE_ACCELERATION, DXL_ID_STEER, profile_acceleration_steering);
            dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID_STEER, profile_velocity_steering);

            currentSpeed = currentSteer = 0;

            unlockMutex();
        }
    }

    void setEnginePower(bool on) {
        if (lockMutex()) {
            if (on) {
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

    void checkTemperature(){
        char buffer[64];
        int t = getEngineSpeedTemperature();
        int t1 = getEngineSteerTemperature();
        
        sprintf(buffer,"Engine Speed Temperature: %d. Engine Steering Temperature: %d", t, t1);
        mqtt::sendTemperature(buffer);

        if(t > 60 || t1 > 60){
            setEnginePower(false);
        
        }


        
 
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
