final int N_STATES_ANGLE = 8; // n states for relative angles
final int N_STATES_DISTANCE = 4; // n states for relative distance

final int N_ACTIONS_STEER = 3;
final int N_ACTIONS_SPEED = 3;
final int N_STATES = N_STATES_ANGLE * N_STATES_DISTANCE;
final int N_ACTIONS = N_ACTIONS_STEER * N_ACTIONS_SPEED;
final int MAX_ACTION = N_ACTIONS - 1;

float EPS = 0.1;

Env env = new Env();
Agent agent = new Agent(EPS);

Observation observation;
Action action;

void setup() {
  size(800, 800);
  observation = env.start();
  action = agent.start(observation);
//  frameRate(5);
}

void draw() {  
  background(255);
  
  // Perform one step.
  observation = env.step(action);
  action = agent.step(observation);
  
  // Draw target.
  noStroke();
  fill(0, 0, 255, 32);
  ellipse(map(env.target.x, 0, 1, 0, width), map(env.target.y, 0, 1, 0, height), 30, 30);

  // Draw agent.
  float x = map(env.position.x, 0, 1, 0, width);
  float y = map(env.position.y, 0, 1, 0, height);
  pushMatrix();
  translate(x, y);
  rotate(env.heading);
  // Draw robot.
//  fill(#8DE869);
  int speed = action.getSpeed();
  if (speed == -1)
    fill(#ff0000);
  else if (speed == 0)
    fill(#ffff00);
  else
    fill(#00ff00);
  ellipse(0, 0, 30, 30);
  // Draw heading indicator.
  fill(0);
  ellipse(15, 0, 5, 5);
  
  popMatrix();
    
  // Reward.
  fill(0);
  text("Reward: " + nf(observation.getReward(), 0, 2), 10, 20);
  text("Speed:  " + action.getSpeed(), 10, 40);
  text("Steer:  " + action.getSteer(), 10, 60);
}

int argmax(float[] vec) {
  int maxIndex = 0;
  float maxValue = vec[0];
  for (int i=1; i<vec.length; i++)
    if (vec[i] > maxValue) {
      maxIndex = i;
      maxValue = vec[i];
    }
  return maxIndex;
}

// Source: https://stackoverflow.com/questions/1878907/the-smallest-difference-between-2-angles
float angleDiff(float a1, float a2) {
  return atan2(sin(a1-a2), cos(a1-a2));
}

float random01() {
  return random(1);
}

int randomInt(int max) {
  return randomInt(0, max);
}

int randomInt(int min, int max) {
  return (int)random(min, max+1);
}

void keyPressed() {
  if (key == ' ')
    env.resetTarget();
}
