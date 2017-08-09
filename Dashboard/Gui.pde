ControlP5 cp5;
RoboBallCanvas robotCanvas;
Slider2D motorSlider;

void createGui() {
  cp5 = new ControlP5(this);
  
  robotCanvas = new RoboBallCanvas(robot, 0, 0, width/3);
//  cp5.addCanvas(robotCanvas);

  Group left = cp5.addGroup("left")
                        .setPosition(0, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(128)
                        .setWidth(width/3);

  Group middle = cp5.addGroup("middle")
                        .setPosition(width/3, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(64)
                        .setWidth(width/3);

  Group right = cp5.addGroup("right")
                        .setPosition(width*2/3, 0)
                        .setBackgroundHeight(height)
                        .setBackgroundColor(0)
                        .setWidth(width/3);
                        
  right.addDrawable(robotCanvas);
}