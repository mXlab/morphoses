class RoboBallValuesCanvas extends Canvas implements CDrawable {
  
  final float ROW_HEIGHT = 32;
  final float PADDING   = 5;
  
  RoboBall robot;
  float x, y, size;
  
  int nRows;

  RoboBallValuesCanvas(RoboBall robot, float x, float y, float size) {
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
    float xPos = x;
    float yPos = y;
 
    pg.beginDraw();
 
    pg.textSize(int(ROW_HEIGHT*.7));
    pg.noStroke();
    
    resetDraw();
    
    // Header.
    drawRow(pg, 32, 255, "PROPERTY", "VALUE", true);

    // Data.
    drawRow(pg, "YAW",   robot.getYaw());
    drawRow(pg, "PITCH", robot.getPitch());
    drawRow(pg, "ROLL",  robot.getRoll());
    drawRow(pg, "ROLLER", robot.getRollerMotorSpeed());
    drawRow(pg, "TILTER", robot.getTilterMotorPosition());
    drawRow(pg, "ROLLER TICKS", robot.getRollerMotorTicks());
    drawRow(pg, "TILTER TICKS", robot.getTilterMotorTicks());

    pg.endDraw();
  }
  
  void resetDraw() {
    nRows = 0;
  }
  
  void drawRow(PGraphics pg, String property, float value) {
    drawRow(pg, nRows % 2 == 0 ? 220 : 200, 0, property, nf(value), false);
  }
  
  void drawRow(PGraphics pg, color bg, color font, String property, String value, boolean header) {
    float columnWidth = size/2;
    float yRow = y+nRows*ROW_HEIGHT;
    pg.fill(bg);
    pg.rect(x, yRow, size, ROW_HEIGHT);
    pg.fill(font);
    pg.textAlign(LEFT, TOP);
    pg.text(property, x+PADDING,             yRow+PADDING);
    if (header) {
      pg.text(value,    x+PADDING+columnWidth, yRow+PADDING);
    }
    else {
      pg.textAlign(RIGHT, TOP);
      pg.text(value,    x+size-PADDING, yRow+PADDING);
    }
    nRows++;
  }

}