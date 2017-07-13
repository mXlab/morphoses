/* mp_tilter_motor Martin Peach 20170109 */
/* based on */
/* mp_roller_motor Martin Peach 20161201 */
/* 20161205 add motor sdpeed control via i2c */
/*
 cytron gearmotor MO-SPG-30E-XXXK with encoder 
*/
/* 20170112: home_tilter resets position to midway between two opto sensors */

#include <Wire.h>
// 10k pullups to 3.3V on the i2c bus are located on the IMU board
// Wire library defaults to enabling pullups to 5V in the Arduino Pro Mini.
// We need to disable the internal pullups.
// digital pins used:
const byte encoderA = 2; // blue wire
const byte encoderB = 3; // violet wire
const byte motorDir = 4; // red wire
const byte motorPWM = 5; // white wire
const byte optoLedL = 6; // green wire to ANODE in LTH1550-01 optodetector via 220Ohm resistor
const byte optoCollL = 7; // blue wire to COLLECTOR of LTH1550-01 optodetector
const byte optoLedR = 8; // green wire to ANODE in LTH1550-01 optodetector via 220Ohm resistor
const byte optoCollR = 9; // blue wire to COLLECTOR of LTH1550-01 optodetector

const byte MOTOR1_I2C_ADDRESS = 8; // i2c address of motor 1 (roller)
const byte MOTOR2_I2C_ADDRESS = 16; // i2c address of motor 2 (tilter)
const byte MOTOR_SPEED = 1; // selector for motor speed
const byte MOTOR_POSITION = 2; // selector for target tick
const byte MOTOR_RESET = 3; // selector to home tilter
const byte MOTOR_MAX_SPEED = 70; // 0-256 maximum speed

// The cytron encoder gives 810 pulses per 360deg rotation with the 1:270 geared motor SPG30E-300K
const int pulsesPerRevolution = 810;

// 'threshold' is the Debounce Adjustment factor for the Rotary Encoder. 
//
// This threshold is the debounce time in microseconds
volatile unsigned long threshold = 1000;


// 'rotaryHalfSteps' is the counter of half-steps. The actual
// number of steps will be equal to rotaryHalfSteps / 2

volatile long rotaryHalfSteps = 0;// Working variables for the interrupt routines
long limitRTicks, limitLTicks;
long actualRotaryTicks = 0;
long targetRotaryTicks = -10;
long previousRotaryTicks = 0;
long deltaTicks = 0;
long targetTick = 0;
boolean fwd;
boolean seeking; // seeking reset position midway between opto sensors
boolean limitL;
boolean limitR;
boolean starting;
boolean do_home_tilter;
int motorSpeed = 0;
int motorMaxSpeed = MOTOR_MAX_SPEED;
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
    motorMaxSpeed = abs(mSpeed)&0x0FFFF;
    Serial.print("NEW motorMaxSpeed ");
    Serial.println(motorMaxSpeed);
  }
  else if (selector == MOTOR_POSITION) {
    long tPos = ((Wire.read()<<24) + (Wire.read()<<16) + (Wire.read()<<8) + (Wire.read())); // receive 4 bytes of a bigendian int
    targetTick = tPos; 
    Serial.print("NEW targetTick ");
    Serial.println(targetTick);
  }
  else if (selector == MOTOR_RESET) {
    Serial.println("RESET");
    seeking = false;
    motorSpeed = 0;
    digitalWrite(motorPWM, motorSpeed);
    do_home_tilter = true;
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
  Wire.begin(MOTOR2_I2C_ADDRESS); // join i2c bus with address #16
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
  
  digitalWrite(optoLedL, LOW);
  pinMode(optoLedL, OUTPUT);
  digitalWrite(optoLedR, LOW);
  pinMode(optoLedR, OUTPUT);
  pinMode(optoCollL, INPUT_PULLUP);
  pinMode(optoCollR, INPUT_PULLUP);

  attachInterrupt(0, int0, CHANGE);
  attachInterrupt(1, int1, CHANGE);
  do_home_tilter = true;
}

void loop () {
  long encoderCount = (rotaryHalfSteps / 2);
  if (do_home_tilter) {
    Serial.write(13);
    home_tilter();
  }
  if (seeking) {
    if (targetTick == encoderCount) {
      seeking = false;
      motorSpeed = 0;
      digitalWrite(motorPWM, motorSpeed);
      digitalWrite(optoLedL, LOW); // off
      digitalWrite(optoLedR, LOW); // off
    }
  }
  else if (targetTick != encoderCount) {
    int deltaTick = ((int)targetTick & 0x0FF) - ((int)encoderCount & 0x0FF);
    Serial.write(13);
    Serial.print("targetTick ");
    Serial.print(targetTick);
    Serial.print(" != encoderCount ");
    Serial.println(encoderCount);
    tilt_to(targetTick); 
  }
  Serial.write('.');
  delay(100);
}

void home_tilter() {
  Serial.println("resetting");
  fwd = seeking = starting = true;
  digitalWrite(motorDir, fwd);
  motorSpeed = 0;
  digitalWrite(motorPWM, motorSpeed);
  digitalWrite(optoLedL, HIGH); // on
  digitalWrite(optoLedR, HIGH); // on
  delay(100);
  rotaryHalfSteps = 0;
  limitRTicks = limitLTicks = 0;

  // motor is off, encoder is zeroed
  while(seeking) {
    
    limitL = (0 ==digitalRead(optoCollL));
    if (limitL) { 
      // we hit the left limit: stop and reverse
      limitLTicks = rotaryHalfSteps / 2;
      Serial.print("limitL Ticks: ");
      Serial.println(limitLTicks);
      Serial.print("starting: ");
      Serial.println(starting);
      if (!starting) fwd = !fwd;
      motorSpeed = 0;
      digitalWrite(motorDir, fwd);
      Serial.print("fwd: ");
      Serial.println(fwd);
      Serial.print("optoCollL: ");
      Serial.println(digitalRead(optoCollL));
      while(0 == digitalRead(optoCollL)) {
        // reverse away from sensor until it reads 'off'
        analogWrite(motorPWM, motorSpeed);
        if (motorSpeed < MOTOR_MAX_SPEED) motorSpeed++;
        delayMicroseconds(250);
      }
      starting = false;
      if (0 != limitRTicks) {
        // we already hit the limit on the other side: go to midway between the limits
        deltaTicks = limitLTicks-limitRTicks;
        Serial.print("1: deltaTicks: ");
        Serial.println(deltaTicks);
        targetRotaryTicks = (limitRTicks + limitLTicks)/2;
        Serial.print("targetRotaryTicks: ");
        Serial.println(targetRotaryTicks);
        // roll until targetRotaryTicks
        do {
          actualRotaryTicks = (rotaryHalfSteps / 2);
          delayMicroseconds(10);
        } while(targetRotaryTicks > actualRotaryTicks);
        // now stop and wait
        motorSpeed = 0;
        analogWrite(motorPWM, motorSpeed);
        Serial.print("1: At target: ");
        actualRotaryTicks = (rotaryHalfSteps / 2);
        Serial.println(actualRotaryTicks);
        seeking = false;
        break;
      }
    }
    limitR = (0 == digitalRead(optoCollR)); // should hit this limit first when moving fwd
    if (limitR) {
      // we hit the right limit: stop and reverse
      limitRTicks = rotaryHalfSteps / 2;
      Serial.print("limitRTicks: ");
      Serial.println(limitRTicks);
      fwd = !fwd;
      motorSpeed = 0;
      digitalWrite(motorDir, fwd);
      Serial.print("fwd: ");
      Serial.println(fwd);
      Serial.print("optoCollR: ");
      Serial.println(digitalRead(optoCollR));
      while(0 == digitalRead(optoCollR)) {
        // reverse away from sensor until it reads 'off'
        analogWrite(motorPWM, motorSpeed);
        if (motorSpeed < MOTOR_MAX_SPEED) motorSpeed++;
        delayMicroseconds(250);
      }
      starting = false;
      if (0 != limitLTicks) {
        // we already hit the limit on the other side: go to midway between the limits
        deltaTicks = limitLTicks-limitRTicks;
        Serial.print("2: deltaTicks: ");
        Serial.println(deltaTicks);
        targetRotaryTicks = (limitRTicks + limitLTicks)/2;
        Serial.print("targetRotaryTicks: ");
        Serial.println(targetRotaryTicks);
        // roll until targetRotaryTicks
        do {
          actualRotaryTicks = (rotaryHalfSteps / 2);
          delayMicroseconds(10);
        } while(targetRotaryTicks < actualRotaryTicks);
        motorSpeed = 0;
        analogWrite(motorPWM, motorSpeed);
        Serial.print("2: At target: ");
        actualRotaryTicks = (rotaryHalfSteps / 2);
        Serial.println(actualRotaryTicks);
        seeking = false;
        break;
      }
    }
    if (motorSpeed < MOTOR_MAX_SPEED) motorSpeed++;
    if (motorSpeed >= 256) digitalWrite(motorPWM, HIGH);
    else analogWrite(motorPWM, motorSpeed);
    digitalWrite(motorDir, fwd);
    delay(10);
  } //-while(seeking)
  digitalWrite(optoLedL, LOW); // off
  digitalWrite(optoLedR, LOW); // off
  rotaryHalfSteps = 0; // reset the tick count
  targetTick = 0; // reset the target
  do_home_tilter = false; // reset completed
}

void tilt_to(int targetTick) {
  actualRotaryTicks = (rotaryHalfSteps / 2);
  Serial.print("tilting to ");
  Serial.println(targetTick);
  seeking = starting = true;
  fwd = (targetTick > actualRotaryTicks); 
  Serial.print("fwd: ");
  Serial.println(fwd);
  digitalWrite(motorDir, fwd);
  motorSpeed = 0;
  digitalWrite(motorPWM, motorSpeed);
  digitalWrite(optoLedL, HIGH); // on
  digitalWrite(optoLedR, HIGH); // on
  delay(100);

  // motor is off, encoder is at whatever it's at
  while(seeking) {
    limitL = (0 == digitalRead(optoCollL));
    if (limitL) {
      if (starting && fwd) {
        // we're at the limit but will move away from it 
        do {
          if (motorSpeed < motorMaxSpeed) motorSpeed++;
          analogWrite(motorPWM, motorSpeed);
          delayMicroseconds(250);
        }while (0 == digitalRead(optoCollL));
        starting = false;
      }
      else {
        // we hit the left limit: stop and return
        limitLTicks = rotaryHalfSteps / 2;
        Serial.print("limitL Ticks: ");
        Serial.println(limitLTicks);
        break;
      }
    }
    limitR = (0 == digitalRead(optoCollR));
    if (limitR) {
      if (starting && !fwd) {
        // we're at the limit but will move away from it 
        do {
          if (motorSpeed < motorMaxSpeed) motorSpeed++;
          analogWrite(motorPWM, motorSpeed);
          delayMicroseconds(250);
        }while (0 == digitalRead(optoCollR));
        starting = false;
      }
      else {
        // we hit the right limit: stop and return
        limitRTicks = rotaryHalfSteps / 2;
        Serial.print("limitR Ticks: ");
        Serial.println(limitRTicks);
        break;
      }
    }
    // roll until targetTick
    if (motorSpeed < motorMaxSpeed) motorSpeed++;
    analogWrite(motorPWM, motorSpeed);
    actualRotaryTicks = (rotaryHalfSteps / 2);
    if (fwd) {
      if (targetTick <= actualRotaryTicks) {
        Serial.print("X");
        break;
      }
    }
    else if (targetTick >= actualRotaryTicks) {
      Serial.print("Y");
      break;
    }
    delayMicroseconds(100);
  }
  // now stop and wait
  motorSpeed = 0;
  analogWrite(motorPWM, motorSpeed);
  Serial.print("At target: actualRotaryTicks:");
  actualRotaryTicks = (rotaryHalfSteps / 2);
  Serial.println(actualRotaryTicks);
  seeking = false;
  digitalWrite(optoLedL, LOW); // off
  digitalWrite(optoLedR, LOW); // off
}

