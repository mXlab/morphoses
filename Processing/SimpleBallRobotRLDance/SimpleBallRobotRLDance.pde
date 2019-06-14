final int N_STATES = 5;
final int N_ACTIONS = 3;
final int MAX_ACTION = N_ACTIONS - 1;

final int N_DANCE_STEPS = 4;
float EPS = 0.05;

Env env = new Env(N_DANCE_STEPS);
Agent agent = new Agent(EPS);

Observation observation;
Action action;

void setup() {
  size(600, 100);
  observation = env.start();
  action = agent.start(observation);
}

void draw() {  
  background(255);
  
  // Perform one step.
  observation = env.step(action);
  action = agent.step(observation);
  
  // Draw target.
  float targetX = map(env.target(), 0, 1, 0, width);
  noStroke();
  fill(0, 0, 255, 32);
  ellipse(targetX, height/2, 30, 30);
  fill(0);
  textAlign(CENTER);
  text(env.danceStep+1, targetX, height/2);

  // Draw agent.
  float x = map(env.x, 0, 1, 0, width);
  fill(#8DE869);
  ellipse(x, height/2, 30, 30);
  
  // Reward.
  text(nf(observation.getReward(), 0, 2), 10, 10);
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
    env.start();
}
