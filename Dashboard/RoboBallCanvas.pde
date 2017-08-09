class RoboBallCanvas extends Canvas implements CDrawable {
  
  RoboBall robot;
  float x, y, size;

  RoboBallCanvas(RoboBall robot, float x, float y, float size) {
    this.robot = robot;
    this.x = x;
    this.y = y;
    this.size = size;
  }
  
  public void setup(PGraphics pg) {
  }

  public void update(PApplet p) {
  }

  public void draw(PGraphics pg) {
    draw(pg, x, y, size);
  }
  
  
  void draw(PGraphics pg, float x, float y, float size) {
    
    // Draw text.
    //scene.stroke(255, 255, 255);
    //scene.strokeWeight(1);
    //scene.textAlign(LEFT);
    //scene.textSize(12);
    //scene.text("YAW:\nPITCH:\nROLL:\n", size*.7, size*.8);
    //scene.text(nf(getYaw(), 3, 2) + "\n" + nf(getPitch(), 3, 2) + "\n" + nf(getRoll(), 3, 2) + "\n", size*.85, size*.8);
   

    float radius = size/2;
    
    PGraphics scene = createGraphics((int)size, (int)size, P3D);
    
//    scene.camera(x, y, y, x, y, 0, 0, 1, 0);
    
    // Draw 3D objects.
    scene.beginDraw();
    {
      scene.background(0);
      scene.noStroke();
      scene.smooth();
      scene.lights();
  
//    scene.camera(x, y, y/ tan(PI*30.0 / 180.0), x, y, 0, 0, 1, 0);
      scene.translate(radius, radius);
      scene.scale(0.7, 0.7);
  
      scene.rotateY(radians(90));
  
      float c1 = cos(radians(-robot.getAdjustedRoll()));
      float s1 = sin(radians(-robot.getAdjustedRoll()));
      float c2 = cos(radians(robot.getAdjustedPitch()));
      float s2 = sin(radians(robot.getAdjustedPitch()));
      float c3 = cos(radians(robot.getAdjustedYaw()));
      float s3 = sin(radians(robot.getAdjustedYaw()));
      scene.applyMatrix( c2*c3, s1*s3+c1*c3*s2, c3*s1*s2-c1*s3, 0,
                   -s2, c1*c2, c2*s1, 0,
                   c2*s3, c1*s2*s3-c3*s1, c1*c3+s1*s2*s3, 0,
                   0, 0, 0, 1);

      // Draw ball.
      scene.strokeWeight(1);
      scene.stroke(255, 255, 255);
      scene.noFill();
      scene.sphere(radius);
      
      // Draw arrows.
      scene.strokeWeight(5);
      
      float arrowHeadLength = 30;
      float arrowLength = radius + arrowHeadLength;
      
      scene.pushMatrix();
  //    rotateY(radians(90));
      scene.stroke(255, 0, 0);
      arrow(scene, 0, 0, arrowLength, 0, arrowHeadLength);
      scene.popMatrix();
      
      scene.pushMatrix();
      scene.rotateY(radians(90));
      scene.stroke(0, 255, 0);
      arrow(scene, 0, 0, arrowLength, 0, arrowHeadLength);
      scene.popMatrix();
  
      scene.pushMatrix();
      scene.rotateZ(-radians(90));
      scene.stroke(0, 0, 255);
      arrow(scene, 0, 0, arrowLength, 0, arrowHeadLength);    
      scene.popMatrix();
    }
    
    scene.endDraw();
    
    /*
    scene.beginDraw();
    scene.lights();
    scene.background(0);
    scene.noStroke();
    scene.translate(width/2, height/2);
    scene.rotateX(frameCount/100.0);
    scene.rotateY(frameCount/200.0);
    scene.box(40);
    scene.endDraw();*/
    
    pg.image(scene, x, y);
  }
  
  void arrow(PGraphics scene, float x1, float y1, float x2, float y2, float len) {
    scene.line(x1, y1, x2, y2);
    
    scene.pushMatrix();
    scene.translate(x2, y2);
    float a = atan2(x1-x2, y2-y1);
    scene.rotate(a);
    scene.line(0, 0, -len, -len);
    scene.line(0, 0, len, -len);
    scene.popMatrix();
  } 

}