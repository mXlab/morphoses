#include "Morphose.h"
#include "Utils.h"

//TODO : MOVE THIS TO Morphose
#define STEER_MAX 0.5f
#define HEADING_FRONT_TOLERANCE 30
const int HEADING_FRONT_MAX = 90 + HEADING_FRONT_TOLERANCE;
const float STEER_HEADING_FRONT_MAX = sin(radians(HEADING_FRONT_MAX));

#define NAVIGATION_ERROR_THRESHOLD 0.2f // threshold above which measurement is considered valid
#define MAX_NAVIGATION_ERROR 45
#define MIN_RELIABLE_NAVIGATION_DISTANCE 0.15
#define MAX_RELIABLE_NAVIGATION_DISTANCE 0.4

const Vec2f REFERENCE_ORIENTATION(1, 0);

float cumulativeNavigationError; // cumulative navigation errors
unsigned int nNavigationSteps;
bool navigationMode;

float targetHeading;
float targetSpeed;

Vec2f startingPosition;
Vec2f velocity;

float velocityHeading;
Chrono velocityTimer;

void startNavigation() {
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

void startNavigationHeading(float speed, float relativeHeading=0) {
  // Get current heading.
  float currentHeading = getHeading();

  // Set target heading.
  targetHeading = - utils::wrapAngle180(currentHeading + relativeHeading);

  // Set target speed.
  targetSpeed = max(speed, 0.0f);

  startNavigation();
}


void stepNavigationHeading() {
  // Check correction. Positive: too much to the left; negative: too much to the right.
  float relativeHeading = utils::wrapAngle180(targetHeading + getHeading());
  float absoluteRelativeHeading = abs(relativeHeading);

  // Compute speed.
  // We use a tolerance in order to force the robot to favor moving forward when it is at almost 90 degrees to avoid situations
  // where it just moves forward and backwards forever. It will move forward  at +- (90 + HEADING_FRONT_TOLERANCE).
  float speed = targetSpeed * (absoluteRelativeHeading < HEADING_FRONT_MAX ? +1 : -1);

  // Compute navigation error.
  float navigationError = (speed > 0 ? absoluteRelativeHeading : 180 - absoluteRelativeHeading);

  // If we are too much away from our direction, reset.
  if (navigationError >= MAX_NAVIGATION_ERROR) {
    startNavigation();
  }
  else {
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
  setEngineSpeed(speed);
  setEngineSteer(steer);
}

// Returns the quality of the velocity calculation from 0% to 100% ie. [0..1]
float getNavigationVelocityQuality() {
  // First part of the error depends on distance moved: longer distances are more reliable.
  float absoluteMovement = velocity.length(); // absolute distance covered
  float movementQuality = mapFloat(absoluteMovement, MIN_RELIABLE_NAVIGATION_DISTANCE, MAX_RELIABLE_NAVIGATION_DISTANCE, 0, 1);
  movementQuality = constrain(movementQuality, 0, 1);

  // Second part of the error depends on average deviation from target during navigation.
  float navigationQuality = (nNavigationSteps > 0 ? mapFloat(cumulativeNavigationError / nNavigationSteps, 0, MAX_NAVIGATION_ERROR, 1, 0) : 0);

  // Return the average of both parts.
  return (movementQuality + navigationQuality) / 2.0f;
}

void stopNavigationHeading() {
  // Update navigation velocity.
  velocity = (morphose::getPosition() - startingPosition);
  velocityHeading = REFERENCE_ORIENTATION.angle(velocity);
  if (!engineIsMovingForward()) velocityHeading = utils::wrapAngle180(velocityHeading + 180);
  
  // Align IMU offset to velocity heading.
  if (getNavigationVelocityQuality() >= NAVIGATION_ERROR_THRESHOLD)
    tare(velocityHeading);

  // Reset engine.
  setEngineSpeed(0);
  setEngineSteer(0);

  // Exit navigation mode.
  navigationMode = false;
  targetHeading = 0;
  targetSpeed = 0;
}

void processNavigation()
{
  if (navigationMode) {
    stepNavigationHeading();
  }
}


Vec2f getVelocity() {
  return velocity;
}

float getVelocityHeading() {
  return velocityHeading;
}

void sendNavigationInfo() {
  bndl.add("/heading").add(getHeading());
  bndl.add("/velocity").add(getVelocity().x).add(getVelocity().y);
  bndl.add("/heading-quality").add(getNavigationVelocityQuality());
}

// 
void updateNavigationVelocity(boolean movingForward) {
}
