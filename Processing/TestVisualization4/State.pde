class State {
  float w[][][][];
  float s1[][];
  float s2[][][];
  
  int time;
  float reward;
  
  State(TableRow row) {
    int c=0;
    time = row.getInt(c++);
    reward = row.getFloat(c++);
    
    // Compute all weights and sums.
    w = new float[N_ACTIONS][N_BINS][N_BINS][N_BINS];
    s2 = new float[N_ACTIONS][N_BINS][N_BINS];
    s1 = new float[N_ACTIONS][N_BINS];
    for (int a=0; a<N_ACTIONS; a++) {
      for (int i=0; i<N_BINS; i++) {
        s1[a][i] = 0;
        for (int j=0; j<N_BINS; j++) {
          s2[a][i][j] = 0;
          for (int k=0; k<N_BINS; k++) {
            float v = row.getFloat(c++);
            s1[a][i] += v;
            s2[a][i][j] += v;
            w[a][i][j][k] = v;
          }
        }
      }
    }
  }
  
  void draw(float x, float y, float len) {
    translate(x, y);
    
    float branchLen = 130;
    int degrees = 315;
    drawAction(0, radians(degrees), branchLen);
    drawAction(1, radians(degrees+=45), branchLen);
    drawAction(2, radians(degrees+=45), branchLen);
    drawAction(5, radians(degrees+=45), branchLen);
    drawAction(8, radians(degrees+=45), branchLen);
    drawAction(7, radians(degrees+=45), branchLen);
    drawAction(6, radians(degrees+=45), branchLen);
    drawAction(3, radians(degrees+=45), branchLen);
    
    drawAction(4, radians(0), branchLen, false);
  }
  
  void drawAction(int a, float angle, float len) {
    drawAction(a, angle, len, true);
  }
  
  void drawAction(int a, float angle, float len, boolean trsl) {
    pushMatrix();
    
    rotate(angle);
    //translate(len, 0);
    
    if (trsl)
      drawBranch(len*2, false);
    
    float sum = 0;
    for (int i=0; i<N_BINS; i++) {
      sum += s1[a][i];
    }

//    len = map(sum, 0, 0.5, 0, 10);
    len = map(sum, -30, 0, len/2, len);

    float[] s1angles = weightsToAngles(s1[a]);
    for (int i=0; i<N_BINS; i++) {
      
      pushMatrix();
      rotate(s1angles[i]);
      drawBranch(len*0.67);

      float[] s2angles = weightsToAngles(s2[a][i]);
      for (int j=0; j<N_BINS; j++) {

        pushMatrix();
        rotate(s2angles[j]);
        drawBranch(len*0.5);

        float[] wangles = weightsToAngles(w[a][i][j]);

        for (int k=0; k<N_BINS; k++) {
          pushMatrix();
          rotate(wangles[k]);
          drawBranch(len*0.33);
          popMatrix();
        }
        
        popMatrix();
      }
      
      popMatrix();
    }
    popMatrix();
  }

  void drawBranch(float len) {
    drawBranch(len, true);
  }
  
  void drawTree(float len) {
    curve(-len, -len,  0, 0, len, len, 0, 3*len);
    if (len > 20) {
      rotate(-len/10);
      drawTree(len * 0.5);

      rotate(len/10);
      drawTree(len * 0.5);
  }
  
  void drawBranch(float len, boolean circle) {
    color baseColor = lerpColor(#ffdbac, #a1665e, constrain(map(len, 15, 45, 0, 1), 0, 1));
    // Branch.
    strokeWeight(1);
//    stroke(0, 8);//#ffdbac, 64);
    stroke(baseColor, 64);
    noFill();
    curve(-len, -len,  0, 0, 0, -len,   -len, -len);
    
//    line(0, 0, 0, -len);

//    line(0, 0, 0, -len);
    // Move to the end of that line
    translate(0, -len);

    if (circle) {
      // External circle.
      int nCurves = round(len/5);
      for (int i=0; i<nCurves; i++) {
        stroke(baseColor, 64);
        rotate(radians(360.0/nCurves));
        drawTree(len);
//        curve(-len, -len,  0, 0, len*0.5, len*0.5, 0, 3*len);
      }
      
  //    fill(baseColor, 64);
  //    strokeWeight(len*0.01);
  //    stroke(0, 8);
  ////    noStroke();
  //    ellipse(0, 0, len*0.5, len*0.5);
      
      // Internal circle.
      fill(baseColor, 128);
      noStroke();
      ellipse(0, 0, len*0.2, len*0.2);
    }
  }

  float[] weightsToAngles(float[] weights) {
    // Compute cumulative sums.
    float[] sum = new float[weights.length];
    sum[0] = weights[0];
    for (int i=1; i<weights.length; i++)
      sum[i] = sum[i-1] + weights[i];
    // Compute and return angles.
    float[] angles = new float[weights.length];
    for (int i=0; i<weights.length; i++)
      angles[i] = map(sum[i], 0, sum[weights.length-1]+1, -PI, PI);
    return angles;
  }
  
  float getWeight(int a, int i, int j, int k) {
    return w[a][i][j][k];
  }
  
  float getSum1(int a, int i) { return s1[a][i]; }

  float getSum2(int a, int i, int j) { return s2[a][i][j]; }

  float getReward() { return reward; }
  
  int getTime() { return time; }
}
