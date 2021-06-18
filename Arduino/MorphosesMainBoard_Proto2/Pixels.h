//const int intPin = 4;  // incoming MPU9250 interrupt

#define RED_LED    5 // red led is also green pin 5 on-board led
#define GREEN_LED 12
#define BLUE_LED  13

const int power = 0; // controls power to the rest of the ball

void initPixels() {
  pinMode(RED_LED,   OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED,  OUTPUT);
}

void setPixels(int r, int g, int b) {
  analogWrite(RED_LED,   r);
  analogWrite(GREEN_LED, g);
  analogWrite(BLUE_LED,  b);
}
