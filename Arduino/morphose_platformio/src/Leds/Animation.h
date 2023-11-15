#ifndef ANIMATION_H
#define ANIMATION_H

#include "Leds/Color.h"
#include "Leds/Pixels.h"
#include "pq_trig8.h"

namespace animations {

// Enum to specify the type of animation
enum AnimationType {
  FULL,
  SIDE,
  CENTER
};

// Animation class definition
class Animation {
  
public:
  // Constructor
  Animation();
  
  // Copy properties from another Animation object
  void copyFrom(const Animation& o);
  
  // Get color for a specific pixel
  Color getColor(int i);
  
  // Set base color
  void setBaseColor(int r, int g, int b);

  // Set alternative color
  void setAltColor(int r, int g, int b);

  // Set noise level
  void setNoise(float noise_);

  // Set the type of animation
  void setType(AnimationType type_);

  // Set period for oscillation
  void setPeriod(float period_);

  // Set region of pixels to affect
  void setRegion(pixels::Region region_);
 
public:
  pixels::Region   region;    // Region of pixels affected
  AnimationType type;      // Type of animation
  float         period;    // Period of oscillation
  bool          isRgb;     // Whether the color is RGB
  float         noise;     // Noise level
  
  Color         baseColor; // Base color
  Color         altColor;  // Alternative color

  pq::SineOsc       osc;       // Sine Oscillator object for animation
};

Animation& currentAnimation();
Animation& previousAnimation();

// Start animation transition timer.
void beginTransition();

// Lock animation mutex.
bool lockMutex();

// Unlock animation mutex.
void unlockMutex();

// Initialize animation.
void initialize();

// Update animation.
void update();

// Display animation.
void display();

// Task handle.
void run(void *parameters);


} // namespace name

#endif // ANIMATION_H
