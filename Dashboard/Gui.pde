ControlP5 cp5;
RoboBallCanvas robotCanvas;

void createGui() {
  cp5 = new ControlP5(this);
  int columnWidth = width/3;
  int borderWidth = 10;
  int columnInternalWidth = columnWidth-2*borderWidth;
  int columnInternalHeight = height-2*borderWidth;
  
  robotCanvas = new RoboBallCanvas(robot, 0, 0, columnInternalWidth);
//  cp5.addCanvas(robotCanvas);

  float columnX = borderWidth;
  Group left = cp5.addGroup("left")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(128)
                        .setWidth(columnInternalWidth);
  columnX += columnWidth;

  // create a toggle
  float x = borderWidth;
  int buttonWidth = 50;
  
  cp5.addToggle("recordData")
     .setPosition(x, borderWidth)
     .setLabel("Record")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth;
  
  cp5.addButton("saveData")     
     .setPosition(x, borderWidth)
     .setLabel("Save")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth;

  cp5.addButton("saveDataAs")     
     .setPosition(x, borderWidth)
     .setLabel("Save as...")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;

  Group middle = cp5.addGroup("middle")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(64)
                        .setWidth(columnInternalWidth);
  columnX+=columnWidth;
  
  Group right = cp5.addGroup("right")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(128)
                        .setWidth(columnInternalWidth);
                        
  cp5.addSlider2D("motorSlider")
     .setPosition(0, 0)
     .setSize(columnInternalWidth, columnInternalWidth)
     .setMinMax(-255, -255, +255, +255)
     .setValue(0, 0)
     .setGroup(middle)
     .setBroadcast(true)
     //.disableCrosshair()
     .moveTo(middle)
     ;

  right.addDrawable(robotCanvas);
}

void motorSlider(float dummy) {
  float[] values = ((Slider2D)cp5.get("motorSlider")).getArrayValue();
  float tilt = values[0];
  float speed = values[1];
  robot.setTilterMotorPosition(round(tilt));
  robot.setRollerMotorSpeed(round(speed));
}

void recordData(boolean rec) {
  logger.setRecording(rec);
}

void saveData() {
  if (outputFileName == null)
    saveDataAs();
  else
    saveOutput();
}

void saveDataAs() {
  selectInput("Save recorded data to...", "setDataFileName");
}

void setDataFileName(File f) {
  if (f == null) {
    println("Window was closed or the user hit cancel.");
  }
  else {
    outputFileName = f.getAbsolutePath();
    saveData();
  }
}