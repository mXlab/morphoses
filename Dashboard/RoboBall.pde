class RoboBall {
  
  // Main.
  int rollerMotorTicks;
  int tilterMotorTicks;
  int rollerMotorSpeed;
  int tilterMotorPosition;
  
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
  
  int getRollerMotorTicks() { return rollerMotorTicks; }
  int getTilterMotorTicks() { return tilterMotorTicks; }

  int getRollerMotorSpeed() { return rollerMotorSpeed; }
  int getTilterMotorPosition() { return tilterMotorPosition; }

  void setRollerMotorTicks(int t) { rollerMotorTicks = t; }
  void setTilterMotorTicks(int t) { tilterMotorTicks = t; }

  void setRollerMotorSpeed(int speed) {
    speed = constrain(speed, -255, 255);
    OscMessage msg = new OscMessage("/motor/1");
    msg.add(speed);
    mainOscP5.send(msg, mainLocation);
    rollerMotorSpeed = speed;
    saveState();
  }

  void setTilterMotorPosition(int pos) {
    pos = constrain(pos, -255, 255);
    OscMessage msg = new OscMessage("/motor/2");
    msg.add(pos);
    mainOscP5.send(msg, mainLocation);
    tilterMotorPosition = pos;
    saveState();
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
  
  void draw() {
    
    // Draw text.
    stroke(255, 255, 255);
    strokeWeight(1);
    textAlign(LEFT);
    textSize(24);
    text("YAW:\nPITCH:\nROLL:\n", width*.7, height*.8);
    text(nf(getYaw(), 3, 2) + "\n" + nf(getPitch(), 3, 2) + "\n" + nf(getRoll(), 3, 2) + "\n", width*.85, height*.8);
   
    noStroke();

    // Draw 3D objects.
    pushMatrix();
    translate(width/2, height/2);
    lights();

    rotateY(radians(90));

    float c1 = cos(radians(-getAdjustedRoll()));
    float s1 = sin(radians(-getAdjustedRoll()));
    float c2 = cos(radians(getAdjustedPitch()));
    float s2 = sin(radians(getAdjustedPitch()));
    float c3 = cos(radians(getAdjustedYaw()));
    float s3 = sin(radians(getAdjustedYaw()));
    applyMatrix( c2*c3, s1*s3+c1*c3*s2, c3*s1*s2-c1*s3, 0,
                 -s2, c1*c2, c2*s1, 0,
                 c2*s3, c1*s2*s3-c3*s1, c1*c3+s1*s2*s3, 0,
                 0, 0, 0, 1);

    // Draw ball.
    strokeWeight(1);
    stroke(255, 255, 255);
    noFill();
    sphere(100);
    
    // Draw arrows.
    strokeWeight(5);
    
    int ARROW_HEAD_LENGTH = 20;
    int ARROW_LENGTH      = 150;
    
    pushMatrix();
//    rotateY(radians(90));
    stroke(255, 0, 0);
    arrow(0, 0, ARROW_LENGTH, 0, ARROW_HEAD_LENGTH);
    popMatrix();
    
    pushMatrix();
    rotateY(radians(90));
    stroke(0, 255, 0);
    arrow(0, 0, ARROW_LENGTH, 0, ARROW_HEAD_LENGTH);
    popMatrix();

    pushMatrix();
    rotateZ(-radians(90));
    stroke(0, 0, 255);
    arrow(0, 0, ARROW_LENGTH, 0, ARROW_HEAD_LENGTH);    
    popMatrix();
    
    stroke(255, 255, 255);
    line(0, 0, 0, mag.x, mag.y, mag.z);
    popMatrix();
  }
  
  void arrow(int x1, int y1, int x2, int y2, float len) {
    line(x1, y1, x2, y2);
    pushMatrix();
    translate(x2, y2);
    float a = atan2(x1-x2, y2-y1);
    rotate(a);
    line(0, 0, -len, -len);
    line(0, 0, len, -len);
    popMatrix();
  } 

  float _wrap(float a, float min, float max) {
    float diff = max-min;
    while (a < min) a += diff;
    while (a > max) a -= diff;
    return a;
  } 
  
}