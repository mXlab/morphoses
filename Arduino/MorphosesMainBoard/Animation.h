#include "Color.h"
#include "pq_trig8.h"

SineOsc osc(1.0);


void runAnimation(void *parameters);

enum AnimationType {
  FULL,
  SIDE,
  CENTER
};

class Animation : public pq::Unit {
  
public:
  Animation(float period_, bool isRgb_=true) : period(period_), isRgb(isRgb_), noiseIsGlobal(false), noise(0), type(FULL) {}

  float oscillator(float offset=0) {
    return osc.shiftBy(offset);//mapTo01( cos( (seconds()+offset) / period * TWO_PI), -1, 1);
  }

  virtual void begin() {
    animationMutex = xSemaphoreCreateMutex();  // crete a mutex object
    
    xTaskCreatePinnedToCore(
      runAnimation, /* Function to implement the task */
      "Animation", /* Name of the task */
      2048,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &taskAnimation,  /* Task handle. */
      0); /* Core where the task should run */
  }

  virtual void step() {
    // Clear all pixels.
    clearPixels();

    float globalNoise = randomFloat(-noise, +noise);

    beginPixelWrite(region);
    
    float offset = 0;
    while (hasNextPixelWrite()) {
      if (type == SIDE)
        offset += period / 8;

      float pixelNoise = (noiseIsGlobal ? globalNoise : randomFloat(-noise, +noise));
      
      Color color = Color::lerp(baseColor, altColor, oscillator(offset) + pixelNoise);
      nextPixelWrite(color.r(), color.g(), color.b());
    }
    
    endPixelWrite();  
  }

  bool lock() {
    return (xSemaphoreTake (animationMutex, portMAX_DELAY));
  }

  void unlock() {
    xSemaphoreGive (animationMutex);  // release the mutex
  }

  void setBaseColor(int r, int g, int b) {
    baseColor.setRgb(r, g, b);
  }

  void setAltColor(int r, int g, int b) {
    altColor.setRgb(r, g, b);
  }

  void setNoise(float noise_, bool noiseIsGlobal_=true) {
    noise = constrain(noise_, 0, 1);
    noiseIsGlobal = noiseIsGlobal_;
  }

  void setType(AnimationType type_) {
    type = type_;
  }

  void setPeriod(float period_) {
      period = period_;
//    osc.period(period_);
  }

  void setRegion(PixelRegion region_) {
    region = region_;
  }
 
public:
  PixelRegion region;
  AnimationType type;
  float period;
  bool    isRgb;

  bool    noiseIsGlobal;
  float   noise;
  
  Color   baseColor;
  Color   altColor;

  TaskHandle_t taskAnimation;
  SemaphoreHandle_t animationMutex = NULL;
};

Animation animation(1.0);

void runAnimation(void *parameters) {
//  animation.begin();
  for (;;) {
    if (animation.lock()) {
      Plaquette.step();
      animation.unlock();
    }
  }
}
