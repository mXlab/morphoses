#include "Color.h"
#include "pq_trig8.h"

enum AnimationType {
  FULL,
  SIDE,
  CENTER
};

Timer transitionTimer(1.0f);

class Animation {
  
public:
  Animation() : region(ALL), type(FULL), period(1), isRgb(true), noise(0), osc(1.0f) {}
  
  void copyFrom(const Animation& o) {
    region = o.region;
    type = o.type;
    period = o.period;
    isRgb = o.isRgb;
    noise = o.noise;
    baseColor = o.baseColor;
    altColor = o.altColor;

    osc.period(period);
  }
  
  Color getColor(int i) {
    if (!pixelIsInsideRegion(i, region))
      return Color(); // black

    float offset = 0;
    if (type == SIDE)
      offset = i / (float)NUM_PIXELS_PER_BLOCK;

    return Color::lerp(baseColor, altColor, (osc.shiftBy(offset) + randomFloat(-noise, +noise)));
  }
  
  void setBaseColor(int r, int g, int b) {
    baseColor.setRgb(r, g, b);
  }

  void setAltColor(int r, int g, int b) {
    altColor.setRgb(r, g, b);
  }

  void setNoise(float noise_) {
    noise = constrain(noise_, 0, 1);
  }

  void setType(AnimationType type_) {
    type = type_;
  }

  void setPeriod(float period_) {
    osc.period(period_);
  }

  void setRegion(PixelRegion region_) {
    region = region_;
  }
 
public:
  PixelRegion   region;
  AnimationType type;

  float   period;
  bool    isRgb;

//  bool    noiseIsGlobal;
  float   noise;
  
  Color   baseColor;
  Color   altColor;

  SineOsc osc;
};

Animation animation;
Animation prevAnimation;

void runAnimation(void *parameters);

TaskHandle_t taskAnimation;
SemaphoreHandle_t animationMutex = NULL;

bool lockAnimationMutex() {
  return (xSemaphoreTake (animationMutex, portMAX_DELAY));
}

void unlockAnimationMutex() {
  xSemaphoreGive (animationMutex);  // release the mutex
}

void initAnimation() {
  // Create mutex.
  animationMutex = xSemaphoreCreateMutex();

  // Create task.
  xTaskCreatePinnedToCore(
    runAnimation, /* Function to implement the task */
    "Animation", /* Name of the task */
    2048,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &taskAnimation,  /* Task handle. */
    0); /* Core where the task should run */  
}

void updateAnimation() {
  //
  pixels.clear();

  for (int i=0; i<pixels.numPixels(); i++) {

    // Get animation color.
    Color color = animation.getColor(i);

    // If in transition: Mix with previous color.
    if (transitionTimer.isStarted() && !transitionTimer.isFinished()) {
      color = Color::lerp( prevAnimation.getColor(i), color, transitionTimer );
    }

    // Set pixel.
    pixels.setPixelColor(i, color.r(), color.g(), color.b());
  }
}

void displayAnimation() {
  pixels.show();
}

void runAnimation(void *parameters) {
  for (;;) {
    if (lockAnimationMutex()) {
      
      Plaquette.step();
      updateAnimation();
      
      unlockAnimationMutex();
      
      displayAnimation();
    }
  }
}
