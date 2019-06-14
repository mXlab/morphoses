class Action {
  int action;
  
  Action(int a) {
    action = constrain(a, 0, MAX_ACTION);
  }
  
  int get() { return action; }
  
  int getX() { return action / N_ACTIONS_X; }
  int getY() { return action % N_ACTIONS_X; }
}
