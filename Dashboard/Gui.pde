ControlP5 cp5;
RoboBallCanvas robotCanvas;

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
  x += buttonWidth+borderWidth;

  cp5.addButton("clearData")
     .setPosition(x, borderWidth)
     .setLabel("Clear data")
     .setWidth(buttonWidth)
     .moveTo(left)
     ;
  x += buttonWidth+borderWidth*2;
  
  cp5.addTextlabel("nData")
     .setPosition(x, borderWidth*2.5)
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

  right.addDrawable(new RoboBallCanvas(robot, 0, 0, columnInternalWidth));
  right.addDrawable(new RoboBallValuesCanvas(robot, 0, columnInternalWidth+borderWidth, columnInternalWidth));
  
  updateDisplay();
}

void updateDisplay() {
  ((Textlabel)cp5.get("nData")).setText("N RECORDS: " + nf(logger.count(), 5, 0));
}

void motorSlider(float dummy) {
  float[] values = ((Slider2D)cp5.get("motorSlider")).getArrayValue();
  int tilt  = round(values[0]);
  int speed = round(values[1]);
  robot.setTilterMotorPosition(tilt);
  robot.setRollerMotorSpeed(speed);
  updateDisplay();
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