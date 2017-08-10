class RoboBall {  

  // Main.
  int rollerMotorTicks;
  int tilterMotorTicks;
  int rollerMotorSpeed;
  int tilterMotorPosition;
  
  boolean power;
  
  // Position.
  float yaw, pitch, roll;
  float yOffset, pOffset, rOffset;
  
  PVector mag;
  
  RoboBall() {
    mag = new PVector();
    reset();
  }

  void reset() {
    setYaw(0);
    setPitch(0);
    setRoll(0);
    rollerMotorTicks = tilterMotorTicks = -1;
    rollerMotorSpeed = tilterMotorPosition = 0;
    power = false;
  }
  
  float getYaw()   { return yaw; }
  float getAdjustedYaw()   { return yaw+yOffset; }
  void setYaw(float yaw) {
    this.yaw   = _wrap(yaw, -180, 180);
  }
  
  float getPitch() { return pitch; }
  float getAdjustedPitch()   { return pitch+pOffset; }
  void setPitch(float pitch) {
    this.pitch = _wrap(pitch, -180, 180);
  }
  
  float getRoll()  { return roll; }
  float getAdjustedRoll()   { return roll+rOffset; }
  void setRoll(float roll) {
    this.roll  = _wrap(roll, -180, 180);
  }
  
  boolean powerIsOn() { return power; }
  void setPower(boolean power) {
    if (this.power != power) {
      this.power = power;
      OscMessage msg = new OscMessage("/power");
      msg.add((int)(power ? 1 : 0));
      mainOscP5.send(msg, mainLocation);
    }
  }
  
  int getRollerMotorTicks() { return rollerMotorTicks; }
  int getTilterMotorTicks() { return tilterMotorTicks; }

  int getRollerMotorSpeed() { return rollerMotorSpeed; }
  int getTilterMotorPosition() { return tilterMotorPosition; }

  void setRollerMotorTicks(int t) { rollerMotorTicks = t; }
  void setTilterMotorTicks(int t) { tilterMotorTicks = t; }

  void setRollerMotorSpeed(int speed) {
    speed = constrain(speed, ROLLER_MIN, ROLLER_MAX);
    if (rollerMotorSpeed != speed) {
      OscMessage msg = new OscMessage("/motor/1");
      msg.add(speed);
      mainOscP5.send(msg, mainLocation);
      rollerMotorSpeed = speed;
      recordState();
    }
  }

  void setTilterMotorPosition(int pos) {
    pos = constrain(pos, TILTER_MIN, TILTER_MAX);
    if (tilterMotorPosition != pos) {
      OscMessage msg = new OscMessage("/motor/2");
      msg.add(pos);
      mainOscP5.send(msg, mainLocation);
      tilterMotorPosition = pos;
      recordState();
    }
  }

  void setMag(float mx, float my, float mz) {
    this.mag.x = mx;
    this.mag.y = my;
    this.mag.z = mz;
  }
  
  void syncOffsets() {
    yOffset = -yaw;
    yaw = 0;
    pOffset = -pitch;
    pitch = 0;
    rOffset = -roll;
    roll = 0;
  }
  
  float _wrap(float a, float min, float max) {
    float diff = max-min;
    while (a < min) a += diff;
    while (a > max) a -= diff;
    return a;
  } 
  
}