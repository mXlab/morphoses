#include "Leds/Animation.h"
using namespace pq;

namespace animations{

Timer transitionTimer(1.0f);
Animation animation;
Animation prevAnimation;

TaskHandle_t taskAnimation;
SemaphoreHandle_t animationMutex = NULL;

bool lockMutex() {
  return (xSemaphoreTake (animationMutex, portMAX_DELAY));
}

void unlockMutex() {
  xSemaphoreGive (animationMutex);  // release the mutex
}

void initialize() {
  // Create mutex.
  animationMutex = xSemaphoreCreateMutex();

  // Create task.
  xTaskCreatePinnedToCore(
    runAnimation, /* Function to implement the task */
    "Animation", /* Name of the task */
    2048,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &taskAnimation,  /* Task handle. */
    0); /* Core where the task should run */  
}



//TODO : RENAME UPDATE LED AND MOVE TO PIXEL
void updateAnimation() {
  //
  pixels::pixels.clear();

  for (int i=0; i<pixels::pixels.numPixels(); i++) {

    // Get animation color.
    Color color = animation.getColor(i);

    // If in transition: Mix with previous color.
    if (transitionTimer.isStarted() && !transitionTimer.isFinished()) {
      color = Color::lerp( prevAnimation.getColor(i), color, transitionTimer );
    }

    // Set pixel.
    pixels::pixels.setPixelColor(i, color.r(), color.g(), color.b());
  }
}

void displayAnimation() {
  pixels::pixels.show();
}

void runAnimation(void *parameters) {
  for (;;) {
    if (lockMutex()) {
      
      Plaquette.step();
      updateAnimation();
      
      unlockMutex();
      
      displayAnimation();
    }
  }
}


}//namespace animations