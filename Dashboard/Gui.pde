ControlP5 cp5;
RoboBallCanvas robotCanvas;
Slider2D motorSlider;

void createGui() {
  cp5 = new ControlP5(this);
  
  robotCanvas = new RoboBallCanvas(robot, (width*2)/3, 20, width/4);
  cp5.addCanvas(robotCanvas);
}