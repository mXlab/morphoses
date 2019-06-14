class Env {
  PVector position;
  float heading; // heading in [0, 2*PI]

  float speedAngular = (TWO_PI / 2) / frameRate;
  float speed = (10.0 / width) / frameRate;
  
  PVector target;
  
  void resetTarget() {
    target = new PVector(random01(), random01());
  }
  
  Observation start() {
    position = new PVector(random01(), random01());
    heading = random(TWO_PI);
    resetTarget();
    return _currentObservation();
  }
  
  Observation step(Action a) {
    // Compute next step.
    if (a.getSpeed() != 0) {
      float currentSpeed = a.getSpeed() * speed;
      heading += a.getSteer() * speedAngular * a.getSpeed();
      while (heading > TWO_PI) heading -= TWO_PI;
      while (heading < 0) heading += TWO_PI;
      position.add(cos(heading)*currentSpeed, sin(heading)*currentSpeed);
      position.x = constrain(position.x, 0, 1);
      position.y = constrain(position.y, 0, 1);
    }
    return _currentObservation();
  }
  
  Observation _currentObservation() {
    float angleDiff = angleDiff(heading, 
                                PVector.sub(target, position).heading());
    return new Observation(map(angleDiff, -PI, PI, -1, 1),
                           map(PVector.dist(position, target), 0, width, 0, 1),
                           rewardFunction(position, heading));
  }
  
  float rewardFunction(PVector position, float heading) {
    return 0.5 - PVector.dist(position, target);
  }
}
