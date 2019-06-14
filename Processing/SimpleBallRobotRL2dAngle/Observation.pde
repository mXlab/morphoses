class Observation extends ArrayList<Float> {
  private float reward;
  
  Observation() {
    this(0, 0, 0);
  }
  
  // Relative angle is in [-1, 1] corresponding to [-180, 180] in degrees.
  // Relative distance is in [0, 1] corresponding to min and max distances.
  Observation(float relativeAngle, float relativeDistance, float r) {
    add(relativeAngle);
    add(relativeDistance);
    reward = r;
  }
  
  int toState() {
    int angleBin = round(map(get(0), -1, 1, 0, N_STATES_ANGLE-1));
    int distanceBin = round(map(get(1), 0, 1, 0, N_STATES_DISTANCE-1));
    println(angleBin + " " + distanceBin);
    return angleBin * N_STATES_DISTANCE + distanceBin;
  }

  public float getReward() { return reward; }
}
