#include "Color.h"
#include "pq_trig8.h"

SineOsc osc(1.0);

class Animation : public pq::Unit {
  
public:
  Animation(float period, bool isRgb_=true) : osc(period), isRgb(isRgb_), noiseIsGlobal(false), noise(0) {}

  virtual void begin() {
  }

  virtual void step() {

    if (noiseIsGlobal) {
      clearPixels();
      Color color = Color::lerp(baseColor, altColor, osc.get() + randomFloat(-noise, noise));
      setPixelsRegion(region, color.r(), color.g(), color.b());      
    }

    else {
      clearPixels();
      beginPixelWrite(region);
      while (hasNextPixelWrite()) {
        Color color = Color::lerp(baseColor, altColor, osc.get() + randomFloat(-noise, noise));
        nextPixelWrite(color.r(), color.g(), color.b());
      }
      endPixelWrite();  
    }
  }

  void setNoise(float noise_) {
    noise = constrain(noise_, 0, 1);
  }

  void setNoiseIsGlobal(bool noiseIsGlobal_) {
    noiseIsGlobal = noiseIsGlobal_;
  }

  void setPeriod(float period_) {
    osc.period(period_);
  }

  void setRegion(PixelRegion region_) {
    region = region_;
  }
 
public:
  pq::SineOsc osc;
  PixelRegion region;
//  uint8_t type;
  bool    isRgb;

  bool    noiseIsGlobal;
  float   noise;
  
  Color   baseColor;
  Color   altColor;
};
