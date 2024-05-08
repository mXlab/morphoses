class Robot { //
  float borderRight;
  float borderLeft;
  float borderTop;
  float borderBottom;
  float borderSpaceX; // space at left & right of the graph
  float borderSpaceY; // space above & below graph
  PImage robotLogo;
  String robotName;
  int robotNumber;
  final int ROBOT_LOGO_SIZE = 100;
  final int ROBOT_LOGO_OFFSET = 50;
  final float ROBOT_LOGO_SHAKE = 1;
  float speed;
  float logoX;
  float logoY;
  float minHeight;
  float maxHeight;
  int nValuesReceived;
  ArrayList<float[]> values;
  float[] valuesTemp = {0, 1, 2, 1, 0, 1, 2, 1, 0, 2, 1, 0, 2, 0, 1};

  //// CONSTRUCTOR ////
  /////////////////////

  Robot(int robotNumber, float borderLeft, float borderRight, float borderTop, float borderBottom) {
    this.borderRight = borderRight;
    this.borderLeft = borderLeft;
    this.borderTop = borderTop;
    this.borderBottom = borderBottom;
    this.robotNumber = robotNumber;
    robotName = "robot" + str(robotNumber);
    this.robotLogo = loadImage(robotName + "_blanc_fond_noir.jpg");




    // values

    values = new ArrayList<float[]>();


    borderSpaceY = (borderBottom - borderTop)/20;

    minHeight = borderTop + ROBOT_LOGO_SIZE + borderSpaceY;
    maxHeight = borderBottom - borderSpaceY;

    logoX = borderRight-ROBOT_LOGO_SIZE-ROBOT_LOGO_OFFSET+random(-ROBOT_LOGO_SHAKE, ROBOT_LOGO_SHAKE);
    logoY = borderTop+ROBOT_LOGO_OFFSET+random(-ROBOT_LOGO_SHAKE, ROBOT_LOGO_SHAKE);
  }

  //// DRAW ////
  //////////////

  void drawLogo() {
    //  float[] lastPoint = values.get(values.size()-1);
    //  tint(getColorReward(lastPoint[lastPoint.length-1]));
    //  tint(getColorReward(0.5));

    image(robotLogo, logoX, logoY, ROBOT_LOGO_SIZE, ROBOT_LOGO_SIZE);
  }

  void drawGraph() {

    /////////////////////

    if (values.size() >= 1) {

      // Get the last point added.
      float[] prevPoint = getY(values, values.size()-1);
      float prevX = borderRight-1;

      // Draw the graph from right to left.
      for (int i=1; i<N_POINTS; i++) {
        // Position of i-th point.
        float x = map(i, 1, N_POINTS-1, borderRight-1, 0); // Inverted: right to left.

        // Get the point to the left.
        int k = (values.size()-1) - i;
        if (0 <= k && k < values.size()) {
          // Draw the line segments for each part of the point.
          float[] point = getY(values, k);
          for (int j=0; j<point.length; j++) {

            // Reward line. //

            if (j == point.length-1) {
              strokeWeight(LINE_WEIGHT_REWARD);
              gradientLine(x, point[j], prevX, prevPoint[j]);
            }

            // Data line. //

            else {
              strokeWeight(LINE_WEIGHT_DATA);
              stroke(COLOR_DATA);
              line(x, point[j], prevX, prevPoint[j]);
            }
          }
          // Update previous point.
          prevPoint = point;
        }
        // Update previous x.
        prevX = x;
      }
    }
  }
  
  void drawGraphTest() {

    /////////////////////

    if (values.size() >= 1) {

      // Get the last point added.
      float[] prevPoint = getY(values, values.size()-1);
      float prevX = borderRight-1;

      // Draw the graph from right to left.
       for (int i=1; i<values.size(); i++) {
        // Position of i-th point.
        float x = i*10; 

        // Get the point to the left.
        //int k = (values.size()-1) - i;
        //if (0 <= k && k < values.size()) {
          // Draw the line segments for each part of the point.
          float[] point = getY(values, i);
          for (int j=0; j<point.length; j++) {

            // Reward line. //

            if (j == point.length-1) {
              strokeWeight(LINE_WEIGHT_REWARD);
              gradientLine(prevX, prevPoint[j],x, point[j]);
            }

            // Data line. //

            else {
              strokeWeight(LINE_WEIGHT_DATA);
              stroke(COLOR_DATA);
              line(x, point[j], prevX, prevPoint[j]);
            }
          }
          // Update previous point.
          prevPoint = point;
        }
        // Update previous x.
        prevX = x;
      }
    }
  }

  // Returns the y coordinates of the i-th data element.

  float[] getY(ArrayList<float[]> array, int i) {
    // Get data.
    float[] point = array.get(i);
    float[] ys = new float[point.length];

    // Map the data to the screen.
    for (int j=0; j<point.length; j++) {
      if (j == point.length - 1) {
        if (minReward == maxReward)
          ys[j] = map(0.5, 0, 1, minHeight, maxHeight);
        else
          ys[j] = map(point[j], minReward, maxReward, minHeight, maxHeight);
      } else
        ys[j] = map(point[j], 0, 1, minHeight, maxHeight);
    }
    // Return the y coordinates.
    return ys;
  }

  
  float[] getYTest(ArrayList<float[]> array, int i) {
    // Get data.
    float[] point = array.get(i);
    float[] ys = new float[point.length];

    // Map the data to the screen.
    for (int j=0; j<point.length; j++) {
      if (j == point.length - 1) {
        if (minReward == maxReward)
          ys[j] = map(0.5, 0, 1, minHeight, maxHeight);
        else
          ys[j] = map(point[j], minReward, maxReward, minHeight, maxHeight);
      } else
        ys[j] = map(point[j], 0, 1, minHeight, maxHeight);
    }
    // Return the y coordinates.
    return ys;
  }


  void gradientLine(float x1, float y1, float x2, float y2) {
    // Calculate the number of steps based on the distance between the points
    int steps = int(dist(x1, y1, x2, y2));

    for (int i = 0; i <= steps; i++) {
      float t = map(i, 0, steps, 0, 1);
      float x = lerp(x1, x2, t);
      float y = lerp(y1, y2, t);

      // Map y position to a value between 0 and 1
      float gradient = map(y, maxHeight, minHeight, 0, 1);

      // Calculate the color for the current step
      int currentColor = lerpColor(COLOR_REWARD_MIN, COLOR_REWARD_MAX, gradient);

      // Set the stroke color
      stroke(currentColor);

      // Draw the point
      point(x, y);
    }
  }

  void addValues(OscMessage msg) {
    if (msg.checkAddrPattern("/" + robotName + "/info")) {
      nValuesReceived++;
      if (nValuesReceived > 5)
        values.add(getOSCValues(msg));
    }
  }

  void addValues(String topic, byte[] payload) {
    if (topic.equals(MQTT_TOPIC_DATA[robotNumber-1])) {
      nValuesReceived++;
      if (nValuesReceived >= 2) // drop the first one
        values.add(getValues(parseJSONArray(new String(payload))));
    }
  }

  void reinitializeValues() {
    nValuesReceived = 0;
    minReward = +9999;
    maxReward = -9999;

    // Reinitialize values.
    values.clear();
  }

  String getName() {
    return robotName;
  }

  int getNumber() {
    return robotNumber;
  }
}
