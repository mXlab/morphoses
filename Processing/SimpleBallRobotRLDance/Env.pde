class Env {
  float x;
  float speed = 0.01;
  
  float[] danceSteps;
  final float THRESHOLD_TOUCHES_TARGET = 0.05;
  int danceStep;
  
  Env(int nDanceSteps) {
    danceSteps = new float[nDanceSteps];
  }
  
  Observation start() {
    x = random01();
    danceStep = 0;
    for (int i=0; i<danceSteps.length; i++)
      danceSteps[i] = random01();
    return _currentObservation();
  }
  
  Observation step(Action a) {
    // Update x.
    int dir = a.get() - 1;
    x += dir * speed;
    x = constrain(x, 0, 1);
    
    // Check if we need to switch dance step.
    if (touchesTarget()) {
      danceStep++;
      danceStep %= danceSteps.length;
    }
    
    return _currentObservation();
  }
  
  Observation _currentObservation() {
    return new Observation(x - target(), _reward());
  }
  
  boolean touchesTarget() {
    return (abs(x - target()) < THRESHOLD_TOUCHES_TARGET);
  }
  
  float target() {
    return danceSteps[danceStep];
  }
  
  float _reward() {
    return 0.5 - abs(x - target());
  }
}
