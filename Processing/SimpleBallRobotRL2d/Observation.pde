class Observation extends ArrayList<Float> {
  private float reward;
  
  Observation() {
    this(0, 0, 0);
  }
  
  Observation(float x, float y, float r) {
    add(x);
    add(y);
    reward = r;
  }

  public float getReward() { return reward; }
}
