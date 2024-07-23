#include "Morphose.h"

#include <Chrono.h>
#include <PlaquetteLib.h>
#include <VectorXf.h>
#include <WiFi.h>

#include "lights/Animation.h"

#include "Utils.h"
#include "communications/Network.h"
#include "communications/asyncMqtt.h"

#include "hardware/Engine.h"
#include "hardware/IMU.h"

#include "lights/Animation.h"
#include "Watchdog.h"

#define AVG_POSITION_TIME_WINDOW 0.2f

namespace morphose {

int id = ROBOT_ID;
bool idle = false;
#if ROBOT_ID == 1
int outgoingPort = 8110;
const char* name = "robot1";
const char* topicName = "morphoses/robot1/data";
#elif ROBOT_ID == 2
int outgoingPort = 8120;
const char* name = "robot2";
const char* topicName = "morphoses/robot2/data";
#elif ROBOT_ID == 3
int outgoingPort = 8130;
const char* name = "robot3";
const char* topicName = "morphoses/robot3/data";
#endif

bool stream = false;
bool sendDataFlag = false;
char jsonString[1024];

Chrono sendRate{true};
Chrono moterCheckTimer{true};
Chrono idleActionTimer{true};

Vec2f currPosition;
pq::Smoother avgPositionX(AVG_POSITION_TIME_WINDOW);
pq::Smoother avgPositionY(AVG_POSITION_TIME_WINDOW);
Vec2f avgPosition;

namespace json {
JsonDocument deviceData;

}

void initialize() {
  Serial.printf("Robot id is set to : %d\n", id);
  Serial.printf("Robot name is : %s\n", name);
  network::outgoingPort =  outgoingPort;  // sets port in network file for desired robot port.
  Serial.printf("Robot streaming port is : %d\n", network::outgoingPort);
  Serial.println("Morphose successfully initialized");
}


void resetPosition() {
  currPosition.set(0, 0);
  avgPosition.set(0, 0);
  avgPositionX.reset();
  avgPositionY.reset();
}

Vec2f getPosition() { return avgPosition; }

void setCurrentPosition(Vec2f newPosition) {
  currPosition.set(newPosition);
  Serial.printf("New position - x : %F y: %F\n", newPosition.x, newPosition.y);
}

void updateLocation() {
  // Update average positioning.

  avgPositionX.put(currPosition.x);
  avgPositionY.put(currPosition.y);
  avgPosition.set(avgPositionX.get(), avgPositionY.get());
}

void setIdle(bool idleMode) {

  
  if (!idle && idleMode) { // when switching to idle

// TODO:    motors::setEngineSpeedPower(0);
    motors::setEngineSpeed(0);
    motors::setEngineSteer(0);

    idleActionTimer.restart();

    if (animations::lockMutex()) {
      animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
      
      animations::currentAnimation().setBaseColor(8, 8, 8);
      animations::currentAnimation().setAltColor(2, 2, 2);
      animations::currentAnimation().setNoise(0.1);
      animations::currentAnimation().setPeriod(6);
      animations::currentAnimation().setType(animations::AnimationType::FULL);
      animations::currentAnimation().setRegion(pixels::Region::TOP);
      animations::beginTransition();  // start transition
      animations::unlockMutex();
    }
  }

  else if (idle && !idleMode) { // when switching from idle
    if (animations::lockMutex()) {
      animations::previousAnimation().copyFrom(animations::currentAnimation());   // save animation
      
      animations::currentAnimation().setBaseColor(0, 0, 0);
      animations::currentAnimation().setAltColor(0, 0, 0);
      animations::currentAnimation().setNoise(0);
      animations::currentAnimation().setPeriod(10);
      animations::currentAnimation().setType(animations::AnimationType::FULL);
      animations::currentAnimation().setRegion(pixels::Region::ALL);
      animations::beginTransition();  // start transition
      animations::unlockMutex();
    }

  }

  // Switch.
  idle = idleMode;
}

void update() {

  if(idle) {
    // Run idle mode.
    if (idleActionTimer.hasPassed(1000, true)) {
      if (pq::randomUniform() < 0.1f) {
        motors::setEngineSteer(pq::randomFloat(-1, 1));
      }
    }
  }else{
    if (sendDataFlag) {
      sendDataFlag = false;

      json::deviceData.clear();

      imus::collectData();

      morphose::navigation::collectData();

      // get data stop here.
      motors::collectData();

      serializeJson(json::deviceData, jsonString);
      // serializeJsonPretty(json::deviceData, Serial);
      mqtt::client.publish(topicName, 0, true, jsonString);
      return;
    }

    updateLocation();

    if (sendRate.hasPassed(STREAM_INTERVAL, true)) {
      imus::process();
      morphose::navigation::process();

      if (stream) {
        sendData();
      }
    }

    if(moterCheckTimer.hasPassed(1000, true)){
        motors::checkTemperature();
        energy::check();  // Energy checkpoint to prevent damage when low
    }

  }

  
}

  

void sendData() { sendDataFlag = true; }

namespace navigation {
#define STEER_MAX 0.5f
#define HEADING_FRONT_TOLERANCE 30
const int HEADING_FRONT_MAX = 90 + HEADING_FRONT_TOLERANCE;
const float STEER_HEADING_FRONT_MAX = sin(radians(HEADING_FRONT_MAX));

#define NAVIGATION_ERROR_THRESHOLD \
  0.2f  // threshold above which measurement is considered valid
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
#if defined(MORPHOSE_DEBUG)
  Serial.printf("Start heading %f %f\n", speed, relativeHeading);
#endif
  float currentHeading = imus::getHeading();

  // Set target heading.
  targetHeading = -utils::wrapAngle180(currentHeading + relativeHeading);

  // Set target speed.
  targetSpeed = max(speed, 0.0f);

  start();
}

void stepHeading() {
  // Check correction. Positive: too much to the left; negative: too much to the
  // right.
  float relativeHeading =
      utils::wrapAngle180(targetHeading + imus::getHeading());
  float absoluteRelativeHeading = abs(relativeHeading);

  // Compute speed.
  // We use a tolerance in order to force the robot to favor moving forward when
  // it is at almost 90 degrees to avoid situations where it just moves forward
  // and backwards forever. It will move forward  at +- (90 +
  // HEADING_FRONT_TOLERANCE).
  float speed =
      targetSpeed * (absoluteRelativeHeading < HEADING_FRONT_MAX ? +1 : -1);

  // Compute navigation error.
  float navigationError =
      (speed > 0 ? absoluteRelativeHeading : 180 - absoluteRelativeHeading);

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
  float steer = steerSign * STEER_MAX *
                constrain(steerValue / STEER_HEADING_FRONT_MAX, 0, 1);

  // Set speed and steer.
  motors::setEngineSpeed(speed);
  motors::setEngineSteer(steer);
}

// Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
float getVelocityQuality() {
  // First part of the error depends on distance moved: longer distances are
  // more reliable.
  float absoluteMovement = velocity.length();  // absolute distance covered
  float movementQuality =
      pq::mapFloat(absoluteMovement, MIN_RELIABLE_NAVIGATION_DISTANCE,
                   MAX_RELIABLE_NAVIGATION_DISTANCE, 0, 1);
  movementQuality = constrain(movementQuality, 0, 1);

  // Second part of the error depends on average deviation from target during
  // navigation.
  float navigationQuality =
      (nNavigationSteps > 0
           ? pq::mapFloat(cumulativeNavigationError / nNavigationSteps, 0,
                          MAX_NAVIGATION_ERROR, 1, 0)
           : 0);

  // Return the average of both parts.
  return (movementQuality + navigationQuality) / 2.0f;
}

void stopHeading() {
  // Update navigation velocity.
  velocity = (morphose::getPosition() - startingPosition);
  velocityHeading = REFERENCE_ORIENTATION.angle(velocity);
  if (!motors::engineIsMovingForward())
    velocityHeading = utils::wrapAngle180(velocityHeading + 180);

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

Vec2f getVelocity() { return velocity; }

float getVelocityHeading() { return velocityHeading; }

void collectData() {
  json::deviceData["heading"] = imus::getHeading();
  json::deviceData["vel-x"] = getVelocity().x;
  json::deviceData["vel-y"] = getVelocity().y;
  json::deviceData["heading-quality"] = getVelocityQuality();
}

}  // namespace navigation

namespace energy {
#define ENERGY_VOLTAGE_LOW_WAKEUP_TIME 10
#define ENERGY_VOLTAGE_OK_MODE 0
#define ENERGY_VOLTAGE_LOW_MODE 1
#define ENERGY_VOLTAGE_CRITICAL_MODE 2

        void deepSleepLowMode(float batteryVoltage) {
            char buff[64];
            sprintf(buff,"Battery low : %.2F volts", batteryVoltage);
            mqtt::debug(buff);
            delay(1000);    // TODO(Etienne): Verify with sofian why delay here

            // Wakeup every ENERGY_VOLTAGE_LOW_WAKEUP_TIME seconds.
            esp_sleep_enable_timer_wakeup(ENERGY_VOLTAGE_LOW_WAKEUP_TIME * 1000000UL);
            mqtt::debug("Battery low2");
            // Go to sleep (light sleep mode).
            esp_light_sleep_start();
        }

        void deepSleepCriticalMode(float batteryVoltage,float batteryVoltageAverage) {
            
            mqtt::sendBatteryCritical();
            char buff[96];
            sprintf(buff,"[Battery critical] Last reading %.2F volts, Average %.2F", batteryVoltage, batteryVoltageAverage);
            mqtt::debug(buff);

            motors::setEnginePower(false);
            // watchdog::deleteCurrentTask();
            delay(1000); 

            // Go to sleep forever (deep sleep mode).
            esp_deep_sleep_start();
        }

        float average(const float* array, const int size) {
          float avg = 0;
          for(int i = 0; i < size; i++){
            avg += array[i];
          }
          return avg /= size;
        }

        void check() {
          static const float criticalVoltage = 12.0;
          static float voltageReading[20]={0};
          static unsigned int voltageReadingIndex = 0;
          static bool firstBufferFill = false;
          
          // Read battery voltage.
          float batteryVoltage = motors::getBatteryVoltage();

          if (batteryVoltage == 0) {
              mqtt::debug("WARNING : Battery missread");
              return;
          }

          voltageReading[voltageReadingIndex] = batteryVoltage;
          voltageReadingIndex = (voltageReadingIndex + 1) % 20; // loop the buffer index
          
          if(voltageReadingIndex == 0) firstBufferFill = true; // Buffer is filled with readings

          if(!firstBufferFill) return; // Wait until buffer is filled
          
          float batteryVoltageMax = *std::max_element(voltageReading, voltageReading + 20);//average(voltageReading , sizeof(voltageReading)/sizeof(voltageReading[0])); // calculate average
              // If energy level is critical, just shut down the ESP.
          if (batteryVoltageMax < criticalVoltage) {
              deepSleepCriticalMode(batteryVoltage,batteryVoltageMax);
          }else{
              mqtt::sendBatteryVoltage(batteryVoltageMax);
              char buff[64];
              sprintf(buff,"Battery voltage : %.2F volts , Max : %.2F", batteryVoltage, batteryVoltageMax);
              mqtt::debug(buff);
          }
        }
    }  // namespace energy
}  // namespace morphose
