class Action {
  int action;
  
  Action(int a) {
    action = constrain(a, 0, MAX_ACTION);
  }
  
  int get() { return action; }
}
