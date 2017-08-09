ControlP5 cp5;
RoboBallCanvas robotCanvas;
Slider2D motorSlider;

void createGui() {
  cp5 = new ControlP5(this);
  int columnWidth = width/3;
  
  robotCanvas = new RoboBallCanvas(robot, 0, 0, columnWidth);
//  cp5.addCanvas(robotCanvas);

  float columnX = 0;
  Group left = cp5.addGroup("left")
                        .setPosition(columnX, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(128)
                        .setWidth(columnWidth);
  columnX+=columnWidth;

  Group middle = cp5.addGroup("middle")
                        .setPosition(columnX, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(64)
                        .setWidth(columnWidth);
  columnX+=columnWidth;
  
  Group right = cp5.addGroup("right")
                        .setPosition(columnX, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(0)
                        .setWidth(columnWidth);
                        
  motorSlider = cp5.addSlider2D("motorSlider")
         .setPosition(0, 0)
         .setSize(columnWidth, columnWidth)
         .setMinMax(-255, -255, +255, +255)
         .setValue(0, 0)
         .setGroup(middle)
         //.disableCrosshair()
         ;

  right.addDrawable(robotCanvas);
}

void motorSlider(float tilt, float speed) {
  robot.setTilterMotorPosition(round(tilt));
  robot.setRollerMotorSpeed(round(speed));
}