class Env {
  PVector position;
  float heading; // heading in [0, 2*PI]

  float speedAngular = TWO_PI/100;
  float speed = 0.1;
  
  PVector target = new PVector(0.5, 0.5);
  
  Observation start() {
    position = new PVector();
    heading = 0;
    return _currentObservation();
  }
  
  Observation step(Action a) {
    // Compute next step.
    heading += speedAngular;
    position.add(cos(heading)*speed, sin(heading)*speed);
    return _currentObservation();
  }
  
  Observation _currentObservation() {
    return new Observation(position, heading, rewardFunction(position, heading));
  }
  
  float rewardFunction(PVector position, float heading) {
    return 0.5 - PVector.dist(position, target);
  }
}
