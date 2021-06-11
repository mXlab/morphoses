#include <Adafruit_NeoPixel.h>

// NEOPIXELs parameters ************************************

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 13
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 4

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//**************************************************************

void initPixels() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)  
}


void setPixels(int r, int g, int b) {
  //neopixel functions:
  pixels.clear(); // Set all pixel colors to 'off'
  for (int i=0; i<pixels.numPixels(); i++)
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  portDISABLE_INTERRUPTS();
  pixels.show();   // Send the updated pixel colors to the hardware.
  portENABLE_INTERRUPTS();
}
