#include <Adafruit_NeoPixel.h>

// NEOPIXELs parameters ************************************

// Which pin on the Arduino is connected to the NeoPixels?
#define PIXELS_PIN 13

Adafruit_NeoPixel pixels(NUM_PIXELS, PIXELS_PIN, PIXELS_TYPE);
//**************************************************************

enum PixelRegion {
  ALL = 0,
  TOP = 1,
  BOTTOM = 2
};

void initPixels() {
  // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.begin(); 
}

// Sets one pixel.
void setPixel(int i, int r, int g, int b, int w=0) {
  pixels.setPixelColor(i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}

// Sets all pixels.
void setPixels(int r, int g, int b, int w=0) {
  // Set all pixel colors to 'off'
  pixels.clear();

  // Write all pixels.
  for (int i=0; i<pixels.numPixels(); i++)
    pixels.setPixelColor(i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}

void clearPixels() {
  pixels.clear();
}

struct PixelIterator {
  PixelRegion region;
  int nextPixel;
  int endPixel;

  PixelIterator(PixelRegion r=ALL) : region(r) {
    nextPixel = 0;
    int nPixels = pixels.numPixels();
    if (region == TOP) {
      nPixels = nPixels * 3 / 4;
    }
    else if (region == BOTTOM) {
      nextPixel = nPixels * 3 / 4;
      nPixels /= 4;
    }

    endPixel = nextPixel + nPixels;
  }

  bool hasNext() const { return nextPixel < endPixel; }
  int  next() { return nextPixel++; }
};

PixelIterator currentRegionIterator;
void beginPixelWrite(PixelRegion region) {
  currentRegionIterator = PixelIterator(region);
}

bool hasNextPixelWrite() { return currentRegionIterator.hasNext(); }
bool nextPixelWrite(int r, int g, int b, int w=0) {
  if (currentRegionIterator.hasNext()) {
    pixels.setPixelColor(currentRegionIterator.next(), pixels.Color(r, g, b, w));
    return true;
  }
  else
    return false;
}

void endPixelWrite() {
  pixels.show();
}

int pixelIsInsideRegion(int i, PixelRegion region) {
  if (i < 0 || i >= pixels.numPixels())
    return false;
  
  if (region == ALL)
    return true;
  else if (region == TOP)
    return (i < pixels.numPixels() * 3 / 4);
  else
    return (i >= pixels.numPixels() * 3 / 4);
}

// Sets pixels in a given region.
void setPixelsRegion(PixelRegion region, int r, int g, int b, int w=0) {
  // Sets values depending on region (default values are for region == ALL).
  PixelIterator it(region);
  while (it.hasNext()) {
    pixels.setPixelColor(it.next(), pixels.Color(r, g, b, w));
  }

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}
