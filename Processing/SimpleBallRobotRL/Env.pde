class Env {
  float x;
  float speed = 0.01;
  float target = 0.8;
  
  Observation start() {
    x = 1;//random01();
    return new Observation(x, rewardFunction(x));
  }
  
  Observation step(Action a) {
    int dir = a.get() - 1;
    x += dir * speed;
    x = constrain(x, 0, 1);
    return new Observation(x, rewardFunction(x));
  }
  
  float rewardFunction(float x) {
    return 0.5 - abs(x - target);
  }
}
