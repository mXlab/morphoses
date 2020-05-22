ControlP5 cp5;
RoboBallCanvas robotCanvas;
Slider2D motorSlider;

void createGui() {
  cp5 = new ControlP5(this);
  int columnWidth = width/3;
  int borderWidth = 10;
  int columnInternalWidth = columnWidth-2*borderWidth;
  int columnInternalHeight = height-2*borderWidth;
  
  float columnX = borderWidth;
  Group left = cp5.addGroup("left")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(128)
                        .setWidth(columnInternalWidth);
  columnX += columnWidth;

  float x = borderWidth;
  float y = borderWidth;
  int buttonWidth = 50;
  
  cp5.addToggle("recordData")
     .setPosition(x, y)
     .setLabel("Record")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth;
  
  cp5.addButton("saveData")     
     .setPosition(x, y)
     .setLabel("Save")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth;

  cp5.addButton("saveDataAs")     
     .setPosition(x, y)
     .setLabel("Save as...")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth;

  cp5.addButton("clearData")
     .setPosition(x, y)
     .setLabel("Clear data")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth*2;
  
  cp5.addTextlabel("nData")
     .setPosition(x, y+borderWidth*1.5)
     ;

  Group middle = cp5.addGroup("middle")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(64)
                        .setWidth(columnInternalWidth);
  columnX+=columnWidth;
  
  x = borderWidth;
  y = borderWidth;
  
  motorSlider =
  cp5.addSlider2D("motorSlider")
     .setPosition(0, 0)
     .setSize(columnInternalWidth, columnInternalWidth)
     .setMinMax(TILTER_MIN, ROLLER_MAX, TILTER_MAX, ROLLER_MIN)
     .setValue(0, 0)
     .setGroup(middle)
     .setBroadcast(true)
     //.disableCrosshair()
     .moveTo(middle)
     ;

  y += columnInternalWidth+borderWidth;
  
  cp5.addToggle("power")
     .setPosition(x, y)
     .setLabel("Power")
     .setSize(buttonWidth, buttonWidth)
     .moveTo(middle)
     ;
  x += buttonWidth+borderWidth;

  cp5.addButton("motorReset")
     .setPosition(x, y)
     .setLabel("Reset")
     .setSize(buttonWidth, buttonWidth)
     .moveTo(middle)
     ;
  x += buttonWidth+borderWidth;

  Group right = cp5.addGroup("right")
                        .setPosition(columnX, borderWidth)
                        .setBackgroundHeight(columnInternalHeight)
                        .setBackgroundColor(128)
                        .setWidth(columnInternalWidth);
                        

  right.addDrawable(new RoboBallCanvas(robot, 0, 0, columnInternalWidth));
  right.addDrawable(new RoboBallValuesCanvas(robot, 0, columnInternalWidth+borderWidth, columnInternalWidth));
  
  updateDisplay();
}

void updateDisplay() {
  ((Textlabel)cp5.get("nData")).setText("N RECORDS: " + nf(logger.count(), 5, 0));
}

void motorSlider(float dummy) {
  float[] values = motorSlider.getArrayValue();
  int tilt  = round(values[0]);
  int speed = round(values[1]);
  robot.setTilterMotorPosition(invertTilter ? - tilt : tilt);
  robot.setRollerMotorSpeed(speed);
  updateDisplay();
}

void power(boolean pow) {
  robot.setPower(pow);
  updateDisplay();
}

void motorReset() {
  motorSlider.setValue(0, 0);
}

void recordData(boolean rec) {
  logger.setRecording(rec);
  updateDisplay();
}

void clearData() {
  recordData(false);
  logger.clear();
  updateDisplay();
}

void saveData() {
  if (outputFileName == null)
    saveDataAs();
  else
    saveOutput();
  updateDisplay();
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