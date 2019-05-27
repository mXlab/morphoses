class Env {
  float x, y;
  float speedX = 0.01;
  float speedY = 0.01;
  float targetX = 0.8;
  float targetY = 0.9;
  
  Observation start() {
    x = y = 0;
    return new Observation(x, y, rewardFunction(x, y));
  }
  
  Observation step(Action a) {
    x += (a.getX() - 1) * speedX;
    x = constrain(x, 0, 1);
    y += (a.getY() - 1) * speedY;
    y = constrain(y, 0, 1);
    return new Observation(x, y, rewardFunction(x, y));
  }
  
  float rewardFunction(float x, float y) {
    return 0.5 - dist(x, y, targetX, targetY);
  }
}
