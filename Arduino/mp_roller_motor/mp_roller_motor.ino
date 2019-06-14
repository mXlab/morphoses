/* mp_roller_motor Martin Peach 20161201 */
/* 20161205 add motor sdpeed control via i2c */
/*
 cytron gearmotor MO-SPG-30E-XXXK with encoder
 with Pololu High-Power Motor Driver 18v15 
*/

#include <Wire.h>
// 10k pullups to 3.3V on the i2c bus are located on the IMU board
// Wire library defaults to enabling pullups to 5V in the Arduino Pro Mini.
// We need to disable the internal pullups.

const byte encoderA = 2; // blue wire
const byte encoderB = 3; // violet wire
const byte motorDir = 4; // red wire
const byte motorPWM = 5; // white wire
const byte MOTOR1_I2C_ADDRESS = 8; // i2c address of motor 1
const byte MOTOR_SPEED = 1; // selector for motor speed

// The cytron encoder gives 810 pulses per 360deg rotation with the 1:270 geared motor SPG30E-300K
const int pulsesPerRevolution = 810;

// 'threshold' is the Debounce Adjustment factor for the Rotary Encoder. 
//
// This threshold is the debounce time in microseconds
volatile unsigned long threshold = 1000;


// 'rotaryHalfSteps' is the counter of half-steps. The actual
// number of steps will be equal to rotaryHalfSteps / 2

volatile long rotaryHalfSteps = 0;// Working variables for the interrupt routines
long actualRotaryTicks = 0;
long previousRotaryTicks = 0;
long deltaTicks = 0;
boolean fwd = true;
boolean speedUp = true;
int motorSpeed = 0;
float rpm = 0;

volatile unsigned long int0time = 0;
volatile unsigned long int1time = 0;
volatile uint8_t int0signal = 0;
volatile uint8_t int1signal = 0;
volatile uint8_t int0history = 0;
volatile uint8_t int1history = 0;

void int0() {
  if ( micros() - int0time < threshold )
    return;
  int0history = int0signal;
  int0signal = bitRead(PIND,2);
  if ( int0history==int0signal )
    return;
  int0time = micros();
  if ( int0signal == int1signal )
    rotaryHalfSteps++;
  else
    rotaryHalfSteps--;
}

void int1() {
  if ( micros() - int1time < threshold )
    return;
  int1history = int1signal;
  int1signal = bitRead(PIND,3);
  if ( int1history==int1signal )
    return;
  int1time = micros();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  Serial.print("wire has ");
  Serial.println(howMany);
  byte selector = Wire.read();
  if (selector == MOTOR_SPEED) {
    long mSpeed = ((Wire.read()<<24) + (Wire.read()<<16) + (Wire.read()<<8) + (Wire.read())); // receive 4 bytes of a bigendian int
    fwd = (mSpeed >= 0);
    motorSpeed = abs(mSpeed)&0x0FFFF;
    if (motorSpeed >= 256) digitalWrite(motorPWM, HIGH);
    else analogWrite(motorPWM, motorSpeed);
    digitalWrite(motorDir, fwd);
  }
  while(Wire.available()) Serial.print(Wire.read(), HEX); // any excess
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  long encoderCount = (rotaryHalfSteps / 2);
  Wire.write(encoderCount>>24); // respond with message of 4 bytes
  Wire.write(encoderCount>>16);
  Wire.write(encoderCount>>8);
  Wire.write(encoderCount);
  // as expected by master
}

void setup() {
  Serial.begin(115200);
  Wire.begin(MOTOR1_I2C_ADDRESS); // join i2c bus with address #8
  digitalWrite(SDA, 0); // Wire.begin() activates the pullups. We need to disable them as soon as possible
  digitalWrite(SCL, 0); // Wire.begin() activates the pullups. We need to disable them as soon as possible
  // i2c voltage exceeds 3V for about 100us
  // we have shottky diodes (1N5818) from esp8266's SDA and SCLK to arduino to prevent 5V entering esp8266
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  pinMode(encoderA, INPUT_PULLUP); // encoder has open-drain putputs
  pinMode(encoderB, INPUT_PULLUP); // encoder has open-drain putputs
  pinMode(motorDir, OUTPUT);
  pinMode(motorPWM, OUTPUT);
  digitalWrite(motorDir, fwd);
  digitalWrite(motorPWM, 0);

  attachInterrupt(0, int0, CHANGE);
  attachInterrupt(1, int1, CHANGE);

}

void loop() {
  actualRotaryTicks = (rotaryHalfSteps / 2);
  deltaTicks = actualRotaryTicks - previousRotaryTicks;
  previousRotaryTicks = actualRotaryTicks;
  
  Serial.print("Speed: ");
  Serial.print(motorSpeed);
  Serial.print(" ticks: ");
  Serial.print(actualRotaryTicks);
  Serial.print(" deltaTicks: ");
  Serial.print(deltaTicks);
  rpm = 60.*(float)deltaTicks/(float)pulsesPerRevolution;
  Serial.print(" rpm: ");
  Serial.println(rpm);
  delay (1000);
}
