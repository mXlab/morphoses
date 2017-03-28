/* 20170116 add /motor/2 position with i2c send*/
/* 20161205 add /motor/1 speed with i2c send*/
/* mp_esp8266MPU9250 was */
/* MPU9250 Basic Example Code
 by: Kris Winer
 date: April 1, 2014
 license: Beerware - Use this code however you'd like. If you
 find it useful you can buy me a beer some time.
 Modified by Brent Wilkins July 19, 2016

 Demonstrate basic MPU-9250 functionality including parameterizing the register
 addresses, initializing the sensor, getting properly scaled accelerometer,
 gyroscope, and magnetometer data out. Added display functions to allow display
 to on breadboard monitor. Addition of 9 DoF sensor fusion using open source
 Madgwick and Mahony filter algorithms. Sketch runs on the 3.3 V 8 MHz Pro Mini
 and the Teensy 3.1.

 SDA and SCL should have external pull-up resistors (to 3.3V).
 10k resistors are on the EMSENSR-9250 breakout board.
 2.2k resistors are on the ESP8266Thing

 Hardware setup:
 MPU9250 Breakout --------- ESP8266Thing
 VDD ---------------------- 3.3V
 VDDI --------------------- 3.3V
 SDA ----------------------- SDA
 SCL ----------------------- SCL
 GND ---------------------- GND
 */

#include "quaternionFilters.h"
#include "MPU9250.h"
#include <OSCBundle.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/*
An attitude and heading reference system (AHRS)
consists of sensors on three axes that provide
attitude information for aircraft, including roll,
pitch and yaw.They are designed to replace
traditional mechanical gyroscopic flight
instruments and provide superior reliability and accuracy.
*/
#define AHRS true         // Set to false for basic data read
#define SerialDebug false  // Set to true to get Serial output for debugging
const bool OSCDebug = true; // debug-print OSC packet stuff
boolean sendOSC = true; // set to true to stream samples
#define useUdp true
#define AP_MODE false

// Pin definitions for ESP8266Thing
const int intPin = 4;  // incoming MPU9250 interrupt
const int redLed  = 5;  // red led is also green pin 5 on-board led
const int greenLed = 12;
const int blueLed = 13;
const int power = 0; // controls power to the rest of the ball
const byte MOTOR1_I2C_ADDRESS = 8; // i2c address of motor 1
const byte MOTOR2_I2C_ADDRESS = 16; // i2c address of motor 2
const byte MOTOR_SPEED = 1; // selector for motor speed
const byte MOTOR_POSITION = 2; // selector for motor encoder position
const byte MOTOR_RESET = 3; // selector for motor home

//#define SEND_DATA_INTERVAL 500
#define SEND_DATA_INTERVAL 100

MPU9250 myIMU;
OSCBundle bndl;
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

float magScale[3] = {0, 0, 0};

const char *ssid = "ball"; // Pointer to the SSID (max 63 char)
const char *password = "roller"; // for WPA2 min 8 char, for open use NULL
unsigned int localPort = 8765; // local port to listen for UDP packets
unsigned int destPort = 8766; // remote port to send UDP packets
IPAddress destIP(192, 168, 4, 2); // remote IP
char packetBuffer[128];

void setup()
{
  pinMode(power, OUTPUT);
  digitalWrite(power, HIGH);

  Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(115200);
  // Set up the interrupt pin, it's set as active high, push-pull
  pinMode(intPin, INPUT); // interrupt out from the IMU
  digitalWrite(intPin, LOW);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, HIGH);

  /**
   * Set up an access point
   * @param ssid          Pointer to the SSID (max 63 char).
   * @param passphrase    (for WPA2 min 8 char, for open use NULL)
   * @param channel       WiFi channel number, 1 - 13.
   * @param ssid_hidden   Network cloaking (0 = broadcast SSID, 1 = hide SSID)
   */
  // now start the wifi
  WiFi.mode(WIFI_AP_STA);
#if AP_MODE
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Can't start softAP");
    while(1); // Loop forever if setup didn't work
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

#else
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress myIP = WiFi.localIP();
  Serial.println(myIP);
#endif

  digitalWrite(greenLed, LOW);


  Serial.println("Starting UDP");
  if (!udp.begin(localPort)) {
    Serial.println("Can't start UDP");
    while(1); // Loop forever if setup didn't work
  }
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  digitalWrite(redLed, LOW);

  WiFi.printDiag(Serial);
  // Read the WHO_AM_I register, this is a good test of communication
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("I AM "); Serial.print(c, HEX);
  Serial.print(" I should be "); Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x68
  {
    Serial.println("MPU9250 is online...");

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.SelfTest);
    Serial.print("x-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[0],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[1],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[2],1); Serial.println("% of factory value");
    Serial.print("x-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[3],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[4],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[5],1); Serial.println("% of factory value");

    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    Serial.println("MPU9250 initialized for active data mode....");

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print("AK8963 "); Serial.print("I AM "); Serial.print(d, HEX);
    Serial.print(" I should be "); Serial.println(0x48, HEX);

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.magCalibration);
    // Initialize device for active mode read of magnetometer
    Serial.println("AK8963 initialized for active data mode....");

    magcalMPU9250(myIMU.magbias, magScale);
    Serial.println("AK8963 mag biases (mG)"); Serial.println(myIMU.magbias[0]); Serial.println(myIMU.magbias[1]); Serial.println(myIMU.magbias[2]);
    Serial.println("AK8963 mag scale (mG)"); Serial.println(magScale[0]); Serial.println(magScale[1]); Serial.println(magScale[2]);
    delay(2000); // add delay to see results before serial spew of data

    if (SerialDebug)
    {
      //  Serial.println("Calibration values: ");
      Serial.print("X-Axis sensitivity adjustment value ");
      Serial.println(myIMU.magCalibration[0], 2);
      Serial.print("Y-Axis sensitivity adjustment value ");
      Serial.println(myIMU.magCalibration[1], 2);
      Serial.print("Z-Axis sensitivity adjustment value ");
      Serial.println(myIMU.magCalibration[2], 2);
    }

  } // if (c == 0x71)
  else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
  digitalWrite(blueLed, LOW);

}

void loop()
{
    // if there's data available, read a packet
  int packetSize = udp.parsePacket();

  if (packetSize)
  {
    if (OSCDebug) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
    }
    IPAddress remote = udp.remoteIP();
    if (OSCDebug) {
      for (int i = 0; i < 4; i++)
      {
        Serial.print(remote[i], DEC);
        if (i < 3)
        {
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(udp.remotePort());
    }
    // read the packet
    OSCMessage messIn;
    while (packetSize--) messIn.fill(udp.read());

    switch(messIn.getError()) {
      case  OSC_OK:
        int messSize;
        if (OSCDebug) Serial.println("no errors in packet");
        messSize = messIn.size();
        if (SerialDebug) {
          Serial.print("messSize: ");
          Serial.println(messSize);
        }
        if (OSCDebug) {
          char addressIn[64];
          messSize = messIn.getAddress(addressIn, 0, 64);
          Serial.print("messSize: ");
          Serial.println(messSize);
          Serial.print("address: ");
          Serial.println(addressIn);
        }
        processMessage(messIn);

        break;
      case BUFFER_FULL:
        if (OSCDebug) Serial.println("BUFFER_FULL error");
        break;
      case INVALID_OSC:
        if (OSCDebug) Serial.println("INVALID_OSC error");
        break;
      case ALLOCFAILED:
        if (OSCDebug) Serial.println("ALLOCFAILED error");
        break;
      case INDEX_OUT_OF_BOUNDS:
        if (OSCDebug) Serial.println("INDEX_OUT_OF_BOUNDS error");
        break;
    }
  } //if (packetSize)

  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0]*myIMU.aRes; // - accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes; // - accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2]*myIMU.aRes; // - accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
    myIMU.getGres();

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0]*myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1]*myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2]*myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
    myIMU.getMres();
    autoCalibrateMagMPU9250();

    Serial.println("bias/scale");
    for (int i=0; i<3; i++) {
      Serial.print(i); Serial.print(" : "); Serial.print(myIMU.magbias[i]); Serial.print(" "); Serial.println(magScale[i]);
    }
    // // User environmental x-axis correction in milliGauss, should be
    // // automatically calculated
    // myIMU.magbias[0] = +470.;
    // // User environmental x-axis correction in milliGauss TODO axis??
    // myIMU.magbias[1] = +120.;
    // // User environmental x-axis correction in milliGauss
    // myIMU.magbias[2] = +125.;

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0] -
                myIMU.magbias[0];
    myIMU.my = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1] -
                myIMU.magbias[1];
    myIMU.mz = (float)myIMU.magCount[2]*myIMU.mRes*myIMU.magCalibration[2] -
                myIMU.magbias[2];
    myIMU.mx *= magScale[0];
    myIMU.my *= magScale[1];
    myIMU.mz *= magScale[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  myIMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  //  MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az,
                         myIMU.gx*DEG_TO_RAD, myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD,
                         myIMU.my, myIMU.mx, myIMU.mz,
                         myIMU.deltat);

  if (!AHRS)
  {
    myIMU.delt_t = millis() - myIMU.count;
    if (myIMU.delt_t > SEND_DATA_INTERVAL)
    {
      if(SerialDebug)
      {
        // Print acceleration values in milligs!
        Serial.print("X-acceleration: "); Serial.print(1000*myIMU.ax);
        Serial.print(" mg ");
        Serial.print("Y-acceleration: "); Serial.print(1000*myIMU.ay);
        Serial.print(" mg ");
        Serial.print("Z-acceleration: "); Serial.print(1000*myIMU.az);
        Serial.println(" mg ");

        // Print gyro values in degree/sec
        Serial.print("X-gyro rate: "); Serial.print(myIMU.gx, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Y-gyro rate: "); Serial.print(myIMU.gy, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Z-gyro rate: "); Serial.print(myIMU.gz, 3);
        Serial.println(" degrees/sec");

        // Print mag values in degree/sec
        Serial.print("X-mag field: "); Serial.print(myIMU.mx);
        Serial.print(" mG ");
        Serial.print("Y-mag field: "); Serial.print(myIMU.my);
        Serial.print(" mG ");
        Serial.print("Z-mag field: "); Serial.print(myIMU.mz);
        Serial.println(" mG");

        myIMU.tempCount = myIMU.readTempData();  // Read the adc values
        // Temperature in degrees Centigrade
        myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
        // Print temperature in degrees Centigrade
        Serial.print("Temperature is ");  Serial.print(myIMU.temperature, 1);
        Serial.println(" degrees C");
      }

      myIMU.count = millis();
      digitalWrite(redLed, !digitalRead(redLed));  // toggle led
    } // if (myIMU.delt_t > SEND_DATA_INTERVAL)
  } // if (!AHRS)
  else
  {
    // Serial print and/or display at 0.5 s rate independent of data rates
    myIMU.delt_t = millis() - myIMU.count;

    // update LCD once per half-second independent of read rate
    if (myIMU.delt_t > SEND_DATA_INTERVAL)
    {

      if (sendOSC) {

        bndl.add("/accel/g").add(myIMU.ax).add(myIMU.ay).add(myIMU.az);
        bndl.add("/gyro/ds").add(myIMU.gx).add(myIMU.gy).add(myIMU.gz);
        bndl.add("/mag/mG").add(myIMU.mx).add(myIMU.my).add(myIMU.mz);
      }

      if(SerialDebug)
      {
        Serial.print("ax = "); Serial.print((int)1000*myIMU.ax);
        Serial.print(" ay = "); Serial.print((int)1000*myIMU.ay);
        Serial.print(" az = "); Serial.print((int)1000*myIMU.az);
        Serial.println(" mg");

        Serial.print("gx = "); Serial.print( myIMU.gx, 2);
        Serial.print(" gy = "); Serial.print( myIMU.gy, 2);
        Serial.print(" gz = "); Serial.print( myIMU.gz, 2);
        Serial.println(" deg/s");

        Serial.print("mx = "); Serial.print( (int)myIMU.mx );
        Serial.print(" my = "); Serial.print( (int)myIMU.my );
        Serial.print(" mz = "); Serial.print( (int)myIMU.mz );
        Serial.println(" mG");

        Serial.print("q0 = "); Serial.print(*getQ());
        Serial.print(" qx = "); Serial.print(*(getQ() + 1));
        Serial.print(" qy = "); Serial.print(*(getQ() + 2));
        Serial.print(" qz = "); Serial.println(*(getQ() + 3));
      }

      // Define output variables from updated quaternion---these are Tait-Bryan
      // angles, commonly used in aircraft orientation. In this coordinate system,
      // the positive z-axis is down toward Earth. Yaw is the angle between Sensor
      // x-axis and Earth magnetic North (or true North if corrected for local
      // declination, looking down on the sensor positive yaw is counterclockwise.
      // Pitch is angle between sensor x-axis and Earth ground plane, toward the
      // Earth is positive, up toward the sky is negative. Roll is angle between
      // sensor y-axis and Earth ground plane, y-axis up is positive roll. These
      // arise from the definition of the homogeneous rotation matrix constructed
      // from quaternions. Tait-Bryan angles as well as Euler angles are
      // non-commutative; that is, the get the correct orientation the rotations
      // must be applied in the correct order which for this configuration is yaw,
      // pitch, and then roll.
      // For more see
      // http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
      // which has additional links.
      float q0 = *getQ(); float q1 = *(getQ()+1); float q2 = *(getQ()+2); float q3 = *(getQ()+3);

			float test = q0*q2 - q3*q1;
			if (test > 0.499f) {
				myIMU.yaw   =   2 * atan2(q0, q3);
				myIMU.pitch =   PI/2;
				myIMU.roll  =   0;
			}
			else if (test < -0.499f) {
				myIMU.yaw   = - 2 * atan2(q0, q3);
				myIMU.pitch = - PI/2;
				myIMU.roll  =   0;
			}
			else {
				myIMU.yaw   =   atan2( 2 * (q0*q3 + q1*q2), 1 - 2 * (sq(q2) + sq(q3)) );
				myIMU.pitch =   asin ( 2 * test );
				myIMU.roll  =   atan2( 2 * (q0*q1 + q2*q3), 1 - 2 * (sq(q1) + sq(q2)) );
			}

      myIMU.yaw   *= RAD_TO_DEG;
      myIMU.pitch *= RAD_TO_DEG;
      myIMU.roll  *= RAD_TO_DEG;

      // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
      // 	8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
      // - http://www.ngdc.noaa.gov/geomag-web/#declination
      //myIMU.yaw   -= 8.5;
      // Declination of Concordia University EV Building is
      // 14.47° W  ± 0.38° on 2017-01-31
      // - http://www.ngdc.noaa.gov/geomag-web/#declination
      myIMU.yaw   -= -14.47;

      if (sendOSC) {
        bndl.add("/ypr/deg").add(myIMU.yaw).add(myIMU.pitch).add(myIMU.roll);
      }
      if(SerialDebug)
      {
        Serial.print("Yaw, Pitch, Roll: ");
        Serial.print(myIMU.yaw, 2);
        Serial.print(", ");
        Serial.print(myIMU.pitch, 2);
        Serial.print(", ");
        Serial.println(myIMU.roll, 2);

        Serial.print("rate = ");
        Serial.print((float)myIMU.sumCount/myIMU.sum, 2);
        Serial.println(" Hz");
      }

      // With these settings the filter is updating at a ~145 Hz rate using the
      // Madgwick scheme and >200 Hz using the Mahony scheme even though the
      // display refreshes at only 2 Hz. The filter update rate is determined
      // mostly by the mathematical steps in the respective algorithms, the
      // processor speed (8 MHz for the 3.3V Pro Mini), and the magnetometer ODR:
      // an ODR of 10 Hz for the magnetometer produce the above rates, maximum
      // magnetometer ODR of 100 Hz produces filter update rates of 36 - 145 and
      // ~38 Hz for the Madgwick and Mahony schemes, respectively. This is
      // presumably because the magnetometer read takes longer than the gyro or
      // accelerometer reads. This filter update rate should be fast enough to
      // maintain accurate platform orientation for stabilization control of a
      // fast-moving robot or quadcopter. Compare to the update rate of 200 Hz
      // produced by the on-board Digital Motion Processor of Invensense's MPU6050
      // 6 DoF and MPU9150 9DoF sensors. The 3.3 V 8 MHz Pro Mini is doing pretty
      // well!

      // get the motor 1 encoder count
      byte incomingCount = Wire.requestFrom((uint8_t)MOTOR1_I2C_ADDRESS, (uint8_t)4);    // request 4 bytes from slave device #8
      byte tick3 = Wire.read();
      byte tick2 = Wire.read();
      byte tick1 = Wire.read();
      byte tick0 = Wire.read();
      if(SerialDebug) {
        Serial.print("received ");
        Serial.print(incomingCount);
        Serial.print(": (3)");
        Serial.print(tick3, HEX);
        Serial.print("(2)");
        Serial.print(tick2, HEX);
        Serial.print("(1)");
        Serial.print(tick1, HEX);
        Serial.print("(0)");
        Serial.println(tick0, HEX);
      }
      int32_t motor1Ticks = (tick3<<24) + (tick2<<16) + (tick1<<8) + tick0;
      if (sendOSC) bndl.add("/motor/1/ticks").add(motor1Ticks);

      // get the motor 2 encoder count
      incomingCount = Wire.requestFrom((uint8_t)MOTOR2_I2C_ADDRESS, (uint8_t)4);    // request 4 bytes from slave device #16
      tick3 = Wire.read();
      tick2 = Wire.read();
      tick1 = Wire.read();
      tick0 = Wire.read();
      if(SerialDebug) {
        Serial.print("received ");
        Serial.print(incomingCount);
        Serial.print(": (3)");
        Serial.print(tick3, HEX);
        Serial.print("(2)");
        Serial.print(tick2, HEX);
        Serial.print("(1)");
        Serial.print(tick1, HEX);
        Serial.print("(0)");
        Serial.println(tick0, HEX);
      }
      int32_t motor2Ticks = (tick3<<24) + (tick2<<16) + (tick1<<8) + tick0;
      if (sendOSC) {
        bndl.add("/motor/2/ticks").add(motor2Ticks);

        if (useUdp) {
          udp.beginPacket(destIP, destPort);
          bndl.send(udp); // send the bytes to the SLIP stream
          udp.endPacket(); // mark the end of the OSC Packet
        }
        else {
          SLIPSerial.beginPacket();
          bndl.send(udp); // send the bytes to the SLIP stream
          SLIPSerial.endPacket(); // mark the end of the OSC Packet
        }
        bndl.empty(); // empty the bundle to free room for a new one
      }
      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } // if (myIMU.delt_t > SEND_DATA_INTERVAL)

  } // if (AHRS)
}

int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};
void autoCalibrateMagMPU9250()
{
//  static int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767};

  // Update mag_max and mag_min.
  for (int i = 0; i < 3; i++) {
    if (myIMU.magCount[i] > mag_max[i]) mag_max[i] = myIMU.magCount[i];
    if (myIMU.magCount[i] < mag_min[i]) mag_min[i] = myIMU.magCount[i];
  }

  // Get hard iron correction.
  float avg_rad = 0;
  for (int i = 0; i < 3; i++) {
    myIMU.magbias[i] = (mag_max[i] + mag_min[i])/2;
    magScale[i]      = (mag_max[i] - mag_min[i])/2;
    avg_rad += magScale[i];
    myIMU.magbias[i] = (float) myIMU.magbias[i]*myIMU.mRes*myIMU.magCalibration[i];  // save mag biases in G for main program
  }
  avg_rad /= 3.0;

  // Get soft iron correction.
  for (int i = 0; i < 3; i++) {
    if (magScale[i] != 0) {
      magScale[i] = avg_rad/((float)magScale[i]);
    }
  }
}

#define Mmode 0x02
void magcalMPU9250(float * dest1, float * dest2)
{
  uint16_t ii = 0, sample_count = 0;

  Serial.println("Mag Calibration: Wave device in a figure eight until done!");
  delay(4000);

  // shoot for ~fifteen seconds of mag data
  if(Mmode == 0x02) sample_count = 128;  // at 8 Hz ODR, new mag data is available every 125 ms
  if(Mmode == 0x06) sample_count = 1500;  // at 100 Hz ODR, new mag data is available every 10 ms
  for(ii = 0; ii < sample_count; ii++) {
    myIMU.readMagData(mag_temp);  // Read the mag data
    for (int jj = 0; jj < 3; jj++) {
      if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
      if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
    }
    if(Mmode == 0x02) delay(135);  // at 8 Hz ODR, new mag data is available every 125 ms
    if(Mmode == 0x06) delay(12);  // at 100 Hz ODR, new mag data is available every 10 ms
  }

  //    Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
  //    Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
  //    Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);

  // Get hard iron correction
  mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
  mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
  mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts

  // Save mag biases in G for main program
  dest1[0] = (float) mag_bias[0]*myIMU.mRes*myIMU.magCalibration[0];
  dest1[1] = (float) mag_bias[1]*myIMU.mRes*myIMU.magCalibration[1];
  dest1[2] = (float) mag_bias[2]*myIMU.mRes*myIMU.magCalibration[2];

  // Get soft iron correction estimate
  mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
  mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
  mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts

  float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
  avg_rad /= 3.0;

  dest2[0] = avg_rad/((float)mag_scale[0]);
  dest2[1] = avg_rad/((float)mag_scale[1]);
  dest2[2] = avg_rad/((float)mag_scale[2]);

  Serial.println("Mag Calibration done!");
}

/// Smart-converts argument from message to integer.
int32_t getArgAsInt(OSCMessage& msg, int index) {
  if (msg.isInt(index))
    return msg.getInt(index);
  else if (msg.isBoolean(index))
    return (msg.getBoolean(index) ? 1 : 0);
  else {
    double val = 0;
    if (msg.isFloat(index))       val = msg.getFloat(index);
    else if (msg.isDouble(index)) val = msg.getDouble(index);
    return round(val);
  }
}

/// Returns true iff argument from message is convertible to a number.
boolean argIsNumber(OSCMessage& msg, int index) {
  return (msg.isInt(index) || msg.isFloat(index) || msg.isDouble(index) || msg.isBoolean(index));
}

void processMessage(OSCMessage& messIn) {
  if (messIn.fullMatch("/stream")) {
    if (OSCDebug) Serial.println("STREAM");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("stream value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      sendOSC = (val != 0);
    }
  }
  else if (messIn.fullMatch("/power")) {
    if (OSCDebug) Serial.println("POWER");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("power value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      digitalWrite(power, val ? LOW : HIGH);
    }
  }
  else if (messIn.fullMatch("/replyto")) {
    if (OSCDebug) Serial.println("REPLYTO");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("reply value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      destIP[3] = val;
    }
  }
  else if (messIn.fullMatch("/motor/1")) {
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("motor 1 value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      char val8 = (char)(val&0xFF);
      Wire.beginTransmission(MOTOR1_I2C_ADDRESS); // transmit to device #8
      Wire.write(MOTOR_SPEED); // sends one byte
      Wire.write(val>>24); // send 4 bytes bigendian 32-bit int
      Wire.write(val>>16);
      Wire.write(val>>8);
      Wire.write(val);
      Wire.endTransmission(); // stop transmitting
    }
  }
  else if (messIn.fullMatch("/motor/2")) {
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("motor 2 value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      char val8 = (char)(val&0xFF);
      Wire.beginTransmission(MOTOR2_I2C_ADDRESS); // transmit to device #8
      Wire.write(MOTOR_POSITION); // sends one byte
      Wire.write(val>>24); // send 4 bytes bigendian 32-bit int
      Wire.write(val>>16);
      Wire.write(val>>8);
      Wire.write(val);
      Wire.endTransmission(); // stop transmitting
    }
  }
  else if (messIn.fullMatch("/reset/2")) {
    // no args
    if (OSCDebug) Serial.println("reset 2");
    Wire.beginTransmission(MOTOR2_I2C_ADDRESS); // transmit to device #8
    Wire.write(MOTOR_RESET); // sends one byte
    Wire.endTransmission(); // stop transmitting
  }
  else if (messIn.fullMatch("/red")) {
    if (OSCDebug) Serial.println("RED");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      //digitalWrite(redLed, (val != 0));
      analogWrite(redLed, (val%256));
    }
  }
  else if (messIn.fullMatch("/green")) {
    if (OSCDebug) Serial.println("GREEN");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      //digitalWrite(greenLed, (val != 0));
      analogWrite(greenLed, (val%256));
    }
  }
  else if (messIn.fullMatch("/blue")) {
    if (OSCDebug) Serial.println("BLUE");
    if (argIsNumber(messIn, 0)) {
      if (OSCDebug) Serial.print("value ");
      int32_t val = getArgAsInt(messIn, 0);
      if (OSCDebug) Serial.println(val);
      //digitalWrite(blueLed, (val != 0));
      analogWrite(blueLed, (val%256));
    }
  }
}
