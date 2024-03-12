#include "Animation.h"

namespace animations {

TaskHandle_t taskAnimation;
SemaphoreHandle_t animationMutex = NULL;

// Current and previous animations.
Animation current;
Animation previous;
int debugColor[4];

pq::Timer transitionTimer(1.0f);

// Constructor to initialize properties
Animation::Animation() : region(pixels::ALL), type(FULL), period(1), isRgb(true), noise(0), osc(1.0f) {}

// Copy properties from another Animation object
void Animation::copyFrom(const Animation& o) {
    region = o.region;
    type = o.type;
    period = o.period;
    isRgb = o.isRgb;
    noise = o.noise;
    baseColor = o.baseColor;
    altColor = o.altColor;
    osc.period(period);
}

// Get color for a specific pixel based on current animation settings
Color Animation::getColor(int i) {
    // Return black if pixel is not in the specified region
    if (!pixels::insideRegion(i, region))
        return Color();

    float offset = 0;
    // Calculate offset for SIDE type animation
    if (type == SIDE)
        offset = i / (float)NUM_PIXELS_PER_BLOCK;

    // Interpolate color based on base and alternative colors
    return Color::lerp(baseColor, altColor, (osc.shiftBy(offset) + pq::randomFloat(-noise, +noise)));
}

// Set the base color
void Animation::setBaseColor(int r, int g, int b) {
    
    baseColor.setRgb(r, g, b);
}

// Set the alternative color
void Animation::setAltColor(int r, int g, int b) {
    altColor.setRgb(r, g, b);
}

// Set the noise level
void Animation::setNoise(float noise_) {
    noise = constrain(noise_, 0, 1);
}

// Set the type of animation (FULL, SIDE, CENTER)
void Animation::setType(AnimationType type_) {
    type = type_;
}

// Set the period for the oscillation
void Animation::setPeriod(float period_) {
    osc.period(period_);
}

// Set the region of pixels to be affected by the animation
void Animation::setRegion(pixels::Region region_) {
    region = region_;
}

Animation& currentAnimation() {
  return current;
}

Animation& previousAnimation() {
  return previous;
}

void beginTransition() {
  transitionTimer.start();
}

void setDebugColor(int r, int g, int b, int w){
    debugColor[0] = r;
    debugColor[1] = g;
    debugColor[2] = b;
    debugColor[3] = w;
}

bool lockMutex() {
  return (xSemaphoreTake (animationMutex, portMAX_DELAY));
}

void unlockMutex() {
  xSemaphoreGive(animationMutex);  // release the mutex
}

void initialize() {
  // Create mutex.
  animationMutex = xSemaphoreCreateMutex();

  // Create task.
  xTaskCreatePinnedToCore(
    run,                  // Function to implement the task
    "Animation",          // Name of the task
    2048,                 // Stack size in words
    NULL,                 // Task input parameter
    0,                    // Priority of the task
    &taskAnimation,       // Task handle.
    0);                   // Core where the task should run
}

void update() {

  // Iterate over all pixels.
  for (int i = 0; i < pixels::numPixels(); i++) {
    // Get animation color.
    Color color = current.getColor(i);

    // If in transition: Mix with previous color.
    if (transitionTimer.isStarted() && !transitionTimer.isFinished()) {
      color = Color::lerp(previous.getColor(i), color, transitionTimer);
    }

    // Set pixel.
    if(COLOR_DEBUGGING == false){
        pixels::set(i, color.r(), color.g(), color.b());
    }
    else {
        pixels::set(
            i,
            debugColor[0],
            debugColor[1],
            debugColor[2],
            debugColor[3]
        );
    }
  }
}

void display() {
  pixels::display();
}

void run(void *parameters) {
  // Infinite loop.
  for (;;) {
    // Wait for mutex.
    if (lockMutex()) {

      // Update animation.
      pq::Plaquette.step();

      update();
      // Unlock mutex.
      unlockMutex();
      // Display animation frame.
      display();
      
    }
  }
}

}  // namespace animations
