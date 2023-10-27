#ifndef MORPHOSE_PIXEL_H
#define MORPHOSE_PIXEL_H

#include <Adafruit_NeoPixel.h>
#include <Globals.h>

// NEOPIXELs parameters ************************************

// Which pin on the Arduino is connected to the NeoPixels?
#define PIXELS_PIN 13

namespace pixels{


extern Adafruit_NeoPixel pixels;
//**************************************************************

enum PixelRegion {
  ALL = 0,
  TOP = 1,
  BOTTOM = 2
};

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

extern PixelIterator currentRegionIterator;
void initPixels() ;

// Sets one pixel.
void setPixel(int i, int r, int g, int b, int w=0);


// Sets all pixels.
void setPixels(int r, int g, int b, int w=0);


void clearPixels() ;




void beginPixelWrite(PixelRegion region);
bool hasNextPixelWrite();
bool nextPixelWrite(int r, int g, int b, int w=0) ;

void endPixelWrite();

int pixelIsInsideRegion(int i, PixelRegion region);

// Sets pixels in a given region.
void setPixelsRegion(PixelRegion region, int r, int g, int b, int w=0) ;



}//namespace pixels


#endif
