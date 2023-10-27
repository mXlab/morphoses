#include "Pixels.h"

namespace pixels{

Adafruit_NeoPixel pixels(NUM_PIXELS, PIXELS_PIN, PIXELS_TYPE);
//**************************************************************
PixelIterator currentRegionIterator;


void initPixels() {
  // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.begin(); 
}

// Sets one pixel.
void setPixel(int i, int r, int g, int b, int w) {
  pixels.setPixelColor(i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}

// Sets all pixels.
void setPixels(int r, int g, int b, int w) {
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




void beginPixelWrite(PixelRegion region) {
  currentRegionIterator = PixelIterator(region);
}

bool hasNextPixelWrite() { return currentRegionIterator.hasNext(); }
bool nextPixelWrite(int r, int g, int b, int w) {
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
void setPixelsRegion(pixels::PixelRegion region, int r, int g, int b, int w) {
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




}//namespace pixels