#include "Pixels.h"
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels.
#define PIXELS_PIN 13

namespace pixels {

  // The neopixel strip instance.
  Adafruit_NeoPixel pixels(NUM_PIXELS, PIXELS_PIN, PIXELS_TYPE);

  void initialize() {
    // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.begin();
  }

  // Sets one pixel.
  void set(int i, int r, int g, int b, int w) {
    pixels.setPixelColor(i, r, g, b, w);
  }

  // // Sets all pixels.
  // void setAll(int r, int g, int b, int w) {
  //   // Set all pixel colors to 'off'
  //   pixels.clear();

  //   // Write all pixels.
  //   for (int i = 0; i < pixels.numPixels(); i++)
  //     pixels.setPixelColor(i, pixels.Color(r, g, b, w));

  // //  portDISABLE_INTERRUPTS();
  //   // Send the updated pixel colors to the hardware.
  //   pixels.show();
  // //  portENABLE_INTERRUPTS();
  // }

  void clear() {
    pixels.clear();
  }

  int numPixels() {
    return pixels.numPixels();
  }

  void display() {
    pixels.show();
  }

struct PixelIterator {
    Region region;
    int nextPixel;
    int endPixel;

    // Constructor.
    PixelIterator(Region r = ALL) : region(r) {
      nextPixel = 0;
      int nPixels = pixels.numPixels();
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
      pixels.setPixelColor(currentRegionIterator.next(), pixels.Color(r, g, b, w));
      return true;
    } else {
      return false;
    }
  }

  void endPixelWrite() {
    pixels.show();
  }

  bool insideRegion(int i, Region region) {
    // Verify index is in range.
    if (i < 0 || i >= pixels.numPixels())
      return false;

    // Verify if index is in region.
    if (region == ALL)
      return true;
    else if (region == TOP)
      return (i < pixels.numPixels() * 3 / 4);
    else
      return (i >= pixels.numPixels() * 3 / 4);
  }

  // Sets pixels in a given region.
  void setRegion(Region region, int r, int g, int b, int w) { // TODO(ETIENNE): Function is not used maybe remove
    // Sets values depending on region (default values are for region == ALL).
    PixelIterator it(region);
    while (it.hasNext()) {
      // Set pixel color for each pixel in region.
      pixels.setPixelColor(it.next(), pixels.Color(r, g, b, w));
    }

  //  portDISABLE_INTERRUPTS();
    // Send the updated pixel colors to the hardware.
    pixels.show();
  //  portENABLE_INTERRUPTS();
  }

}  // namespace pixels
