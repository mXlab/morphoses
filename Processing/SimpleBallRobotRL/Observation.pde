class Observation extends ArrayList<Float> {
  private float reward;
  
  Observation() {
    this(0, 0);
  }
  
  Observation(float x, float r) {
    add(x);
    reward = r;
  }

  public float getReward() { return reward; }
}
