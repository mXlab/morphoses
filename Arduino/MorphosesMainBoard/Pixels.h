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

// Sets pixels in a given region.
void setPixelsRegion(PixelRegion region, int r, int g, int b, int w=0) {
  // Sets values depending on region (default values are for region == ALL).
  int firstPixel = 0;
  int nPixels = pixels.numPixels();
  if (region == TOP) {
    nPixels = nPixels * 3 / 4;
  }
  else if (region == BOTTOM) {
    firstPixel = nPixels * 3 / 4;
    nPixels /= 4;
  }
  
  for (int i=0; i<nPixels; i++)
    pixels.setPixelColor(firstPixel + i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}
