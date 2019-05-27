class Action {
  int action;
  
  Action(int a) {
    action = constrain(a, 0, MAX_ACTION);
  }
  
  int get() { return action; }
  
  int getSteer() { return (action / N_ACTIONS_STEER) - 1; }
  int getSpeed() { return (action % N_ACTIONS_STEER) - 1; }
}
