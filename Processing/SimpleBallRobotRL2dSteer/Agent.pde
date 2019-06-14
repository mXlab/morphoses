
class Agent {
  float epsilon;
  float gamma;
  float learningRate;
  
  float Q[][];
  
  int prevA;
  int prevS;

  Agent(float eps) {
   Q = new float[N_STATES][N_ACTIONS];
   epsilon = eps;
   gamma = 0.99;
   learningRate = 0.01;
  }
  
  Action start(Observation o) {
    // Initialize Q values.
    for (int s=0; s<N_STATES; s++)
      for (int a=0; a<N_ACTIONS; a++)
        Q[s][a] = random(0, 1);
    
    // Get state.
    int s = toState(o);
    
    // Choose action based on policy.
    prevA = chooseAction(s);
    prevS = s;
    
    return new Action(prevA);
  }
  
  Action step(Observation o) {
    // Get reward.
    float r = o.getReward();

    // Adjust Q(s, a) based on results.
    println("state[" + prevS + "]");
    println(Q[prevS]);
    Q[prevS][prevA] += r - gamma * (Q[prevS][prevA] - learningRate * Q[prevS][argmax(Q[prevS])]);
//    Q[prevS][prevA] += gamma * (r - learningRate * (Q[prevS][prevA] - Q[prevS][argmax(Q[prevS])]));

    // Get state.
    int s = toState(o);

    // Choose action based on policy.
    prevA = chooseAction(s);
    prevS = s;
    
    return new Action(prevA);
  }
  
  int chooseAction(int s) {
    if (random01() < epsilon)
      return randomInt(MAX_ACTION);
    else
      return argmax(Q[s]);
  }
  
  int toState(Observation o) {
    
    return (int)constrain( map(o.get(0), 0, 1, 0, N_STATES), 0, N_STATES-1);
  }
}
