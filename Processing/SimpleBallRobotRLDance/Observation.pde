class Observation extends ArrayList<Float> {
  private float reward;
  
  Observation() {
    this(0, 0);
  }
  
  Observation(float xTargetDiff, float r) {
    add(xTargetDiff);
    reward = r;
  }

  public float getReward() { return reward; }
  
  int toState() {
    return round(map(get(0), -1, 1, 0, N_STATES-1));
  }

}
