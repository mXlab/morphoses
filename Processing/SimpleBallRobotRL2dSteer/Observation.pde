class Observation extends ArrayList<Float> {
  private float reward;
  
  Observation() {
    this(new PVector(), 0, 0);
  }
  
  Observation(PVector position, float h, float r) {
    add(position.x);
    add(position.y);
    add(h);
    reward = r;
  }
  
  int toState() {
    // Not finished
    int s = N_STATES_X * (N_STATES_Y * get(0) + get(1)) + get(2);
    return s;
  }

  public float getReward() { return reward; }
}
