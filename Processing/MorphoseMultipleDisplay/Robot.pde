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
  float robotLogoSize;
  final int ROBOT_LOGO_OFFSET = 50;
  final float ROBOT_LOGO_SHAKE = 1;
  //float speed;
  float logoX;
  float logoY;
  float minHeight;
  float maxHeight;
  int nValuesReceived;
  ArrayList<float[]> values;
  float[] valuesTemp = {0, 1, 2, 1, 0, 1, 2, 1, 0, 2, 1, 0, 2, 0, 1};
  PGraphics graph;
  float dataStep;
  float oldNValues = 0;

  float timeBetweenValues;

  //float minReward;
  //float maxReward;

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

    graph = createGraphics(20000, int(borderBottom-borderTop));
    timeBetweenValues = 0;

    // values

    values = new ArrayList<float[]>();

    borderSpaceY = (borderBottom - borderTop)/20;

    robotLogoSize = height/5;
    logoX = borderRight-robotLogoSize;
    logoY = borderTop+ROBOT_LOGO_OFFSET+random(-ROBOT_LOGO_SHAKE, ROBOT_LOGO_SHAKE);

    minHeight = robotLogoSize + borderSpaceY;
    maxHeight = graph.height - borderSpaceY;

    //minReward = -9999;
    //maxReward = 9999;

    dataStep = 10;
  }

  //// DRAW ////
  //////////////

  void drawLogo() {
    fill (0);
    rect(0, borderRight - robotLogoSize, robotLogoSize, robotLogoSize);
    image(robotLogo, logoX, logoY, robotLogoSize, robotLogoSize);
  }

  void drawGraph() {
    graph.beginDraw();
    graph.background(0);
    if (values.size() >= 1) {
      float[] prevPoint = getY(values, 0);
      float prevX = borderLeft;
      // Draw the graph
      for (int i=1; i <= values.size()-1; i++) {
        // Position of i-th point.
        float x = (i+1)*dataStep;

        // Draw the line segments for each part of the point.
        float[] pointY = getY(values, i);
        for (int j=0; j<pointY.length; j++) {

          // Reward line. //

          if (j == pointY.length-1 && j>0) {
            graph.stroke(255, 0, 0);
            graph.strokeWeight(LINE_WEIGHT_REWARD);
            gradientLine( prevX, prevPoint[j], x, pointY[j]);

            float valuesReward[] = values.get(i);

            graph.text(str(valuesReward[valuesReward.length-1]), x, pointY[j] -10);
          }

          // Data line. //

          else {
            graph.strokeWeight(LINE_WEIGHT_DATA);
            graph.stroke(COLOR_DATA);
            graph.line(prevX, prevPoint[j], x, pointY[j]);
          }
        }
        // Update previous point.
        prevPoint = pointY;

        // Update previous x.
        prevX = x;
      }

      // draw rectangle to hide end
      graph.endDraw();
      image(graph, 0, borderTop);
    }
  }

  //// METHODS ////
  /////////////////

  // Returns the y coordinates of the i-th data element.

  //void computeMinMaxOSC(float nValues, float i, float oscValuesI) {
  //  if (i == nValues - 1) {
  //    minReward = min(minReward, oscValuesI);
  //    maxReward = max(maxReward, oscValuesI);
  //    //println(i);
  //  }
  //}

  // draw //
  //////////


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
      graph.stroke(currentColor);

      // Draw the point
      graph.point(x, y);
    }
  }

  // values //
  ////////////

  float[] getValues(JSONArray array) {
    // Get n. values.
    int nValues = array.size();

    // Extract values.
    float[] values = new float[nValues];
    for (int i=0; i<nValues; i++) {
      values[i] = array.getFloat(i);

      // Compute min/max reward values.
      if (i == nValues - 1) {
        minReward = min(minReward, values[i]);
        maxReward = max(maxReward, values[i]);
      }
    }
    // Return values.
    return values;
  }

  float[] getY(ArrayList<float[]> array, int i) {
    // Get data.
    float[] point = array.get(i);
    float[] ys = new float[point.length];

    // Map the data to the screen.
    for (int j=0; j<point.length; j++) {
      if (j == point.length - 1) {
        if (minReward == maxReward)
          ys[j] = map(0.5, 0, 1, maxHeight, minHeight);
        else
          ys[j] = map(point[j], minReward, maxReward, maxHeight, minHeight);
      } else
        ys[j] = map(point[j], 0, 1, maxHeight, minHeight);
    }
    // Return the y coordinates.
    return ys;
  }

  void addValues(OscMessage msg) {
    if (msg.checkAddrPattern("/" + robotName + "/info")) {
      if (nValuesReceived != oldNValues) {

        //timeBetweenValues = float(valuesWatch);
        valuesWatch.start();
      }
      println(valuesWatch.second());
      nValuesReceived++;
      oldNValues = nValuesReceived;
      
      if (nValuesReceived > 5) {
        values.add(getOSCValues(msg));
        speed = getSpeed();
      }
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
    speed = 0;

    // Reinitialize values.
    values.clear();
  }

  // communicate with main code //
  ////////////////////////////////

  String getName() {
    return robotName;
  }

  int getNumber() {
    return robotNumber;
  }
}
