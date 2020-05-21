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
    
    for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        int a = i*3+j;
        draw(a, map(i, 0, 2, width/6, 5*width/6), map(j, 0, 2, height/6, 5*height/6), width/8);
      }
    }

  }
  
  void draw(int a, float x, float y, float len) {
    pushMatrix();

    translate(x, y);
//    drawBranch(len);
    float sum = 0;
    for (int i=0; i<N_BINS; i++) {
      sum += s1[a][i];
    }
    
//    len = map(sum, 0, 0.5, 0, 10);
    len = map(sum, -5, 0, 0, 10);

    float[] s1angles = weightsToAngles(s1[a]);
    for (int i=0; i<N_BINS; i++) {
      
      pushMatrix();
      rotate(s1angles[i]);
      drawBranch(len);

      float[] s2angles = weightsToAngles(s2[a][i]);
      for (int j=0; j<N_BINS; j++) {

        pushMatrix();
        rotate(s2angles[j]);
        drawBranch(len*0.67);

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
    strokeWeight(0.5);
    stroke(0, 128);
    line(0, 0, 0, -len);
    fill(255, 0);
    ellipse(0, 0, len/2, len/2);
//    line(0, 0, 0, -len);
    // Move to the end of that line
    translate(0, -len);
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
