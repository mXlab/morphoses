final int N_STATES = 9;
final int N_ACTIONS = 3;
final int MAX_ACTION = N_ACTIONS - 1;

float EPS = 0.1;

Env env = new Env();
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
  
  // Draw agent.
  float x = map(observation.get(0), 0, 1, 0, width);
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
