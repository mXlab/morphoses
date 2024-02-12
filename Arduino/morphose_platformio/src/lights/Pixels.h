#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_PIXELS_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_PIXELS_H_

#include <Globals.h>

namespace pixels {

enum Region {
  ALL = 0,
  TOP = 1,
  BOTTOM = 2
};

// Function declarations
void init();

// Set one pixel.
void set(int i, int r, int g, int b, int w = 0);

// Set all pixels.
void setAll(int r, int g, int b, int w = 0);

// Clear all pixels.
void clear();

// Get number of pixels.
int numPixels();

// Display all pixels.
void display();

// Returns true iff LED i is inside given region.
bool insideRegion(int i, Region region);

// Set the color of a region.
void setRegion(Region region, int r, int g, int b, int w = 0);

}  // namespace pixels


#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_PIXELS_H_
