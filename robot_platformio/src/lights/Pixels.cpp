#include "Pixels.h"

// Which pin on the Arduino is connected to the NeoPixels.
#define PIXELS_PIN 13

namespace pixels {

  
  CRGBW leds[NUM_PIXELS];
  CRGB *ledsRGB = (CRGB *) &leds[0];
  void initialize() {
    
    FastLED.addLeds<LED_TYPE, PIXELS_PIN, COLOR_ORDER>(ledsRGB, getRGBWsize(NUM_PIXELS));
    for(int i = 0 ; i < NUM_PIXELS; i++){
      leds[i] = CRGBW(0,0,0,100);
    }
    FastLED.show();
    delay(500);
    FastLED.clear();
    FastLED.show();
  }

  // Sets one pixel.
  void set(int i, int r, int g, int b, int w) {

    leds[i] = CRGBW(r,g,b,w);

  }


  void clear() {
    FastLED.clear();
  }

  int numPixels() {
    return NUM_PIXELS;
  }

  void display() {
    FastLED.show();
  }

struct PixelIterator {
    Region region;
    int nextPixel;
    int endPixel;

    // Constructor.
    PixelIterator(Region r = ALL) : region(r) {
      nextPixel = 0;
      int nPixels = NUM_PIXELS;
      if (region == TOP) {
        nPixels = nPixels * 3 / 4;
      } else if (region == BOTTOM) {
        nextPixel = nPixels * 3 / 4;
        nPixels /= 4;
      }
      endPixel = nextPixel + nPixels;
    }

    // Iterator methods.
    bool hasNext() const { return nextPixel < endPixel; }
    int  next() { return nextPixel++; }
  };

  // Iterator for current region.
  PixelIterator currentRegionIterator;

  void beginPixelWrite(Region region) {
    currentRegionIterator = PixelIterator(region);
  }

  bool hasNextPixelWrite() { return currentRegionIterator.hasNext(); }

  bool nextPixelWrite(int r, int g, int b, int w) {
    // Set pixel color for next pixel in region.
    if (currentRegionIterator.hasNext()) {
      leds[currentRegionIterator.next()] = CRGBW(r, g, b, w);
      return true;
    } else {
      return false;
    }
  }

  void endPixelWrite() {
    FastLED.show();
  }

  bool insideRegion(int i, Region region) {
    // Verify index is in range.
    if (i < 0 || i >= NUM_PIXELS)
      return false;

    // Verify if index is in region.
    if (region == ALL)
      return true;
    else if (region == TOP)
      return (i < NUM_PIXELS * 3 / 4);
    else
      return (i >= NUM_PIXELS * 3 / 4);
  }

  // Sets pixels in a given region.
  void setRegion(Region region, int r, int g, int b, int w) { // TODO(ETIENNE): Function is not used maybe remove
    // Sets values depending on region (default values are for region == ALL).
    PixelIterator it(region);
    while (it.hasNext()) {
      // Set pixel color for each pixel in region.
      leds[it.next()] = CRGBW(r, g, b, w);
    }

    // Send the updated pixel colors to the hardware.
    FastLED.show();

  }

}  // namespace pixels
