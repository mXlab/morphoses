#include <Adafruit_NeoPixel.h>

// NEOPIXELs parameters ************************************

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 13
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 4

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//**************************************************************

void initPixels() {
  // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.begin(); 
}

void setPixel(int i, int r, int g, int b, int w=0) {
  pixels.setPixelColor(i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}

void setPixels(int r, int g, int b, int w=0) {
  // Set all pixel colors to 'off'
  pixels.clear();

  // Write all pixels.
  for (int i=0; i<pixels.numPixels(); i++)
//    if (i != 6 && i != 7)
    pixels.setPixelColor(i, pixels.Color(r, g, b, w));

//  portDISABLE_INTERRUPTS();  
  // Send the updated pixel colors to the hardware.
  pixels.show();   
//  portENABLE_INTERRUPTS();
}
