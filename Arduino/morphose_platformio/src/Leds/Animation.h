#ifndef ANIMATION_H
#define ANIMATION_H

#include "Globals.h"
#include "Color.h"
#include "pq_trig8.h"
#include "Pixels.h"
#include <PlaquetteLib.h>
using namespace pq;

enum AnimationType {
  FULL,
  SIDE,
  CENTER
};



class Animation {
  
public:
  Animation() : region(pixels::ALL), type(FULL), period(1), isRgb(true), noise(0), osc(1.0f) {}
  
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

  void setRegion(pixels::PixelRegion region_) {
    region = region_;
  }
 
public:
  pixels::PixelRegion   region;
  AnimationType type;

  float   period;
  bool    isRgb;

//  bool    noiseIsGlobal;
  float   noise;
  
  Color   baseColor;
  Color   altColor;

  SineOsc osc;
};

namespace animations{
extern Animation animation;
extern Animation prevAnimation;
extern Timer transitionTimer;

void runAnimation(void *parameters);


bool lockMutex();

void unlockMutex();

void initialize();

void update();

void updateAnimation();

void displayAnimation();


}//namespace animations



#endif