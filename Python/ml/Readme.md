# Experiments

Here, I report experiments made with the ball and specific RL formalizations/algorithms/parameterizations, to be able to reproduce them later.

# 1st experiment

### Goal behaviour

Stabilize into a static posture.

### Algorithm

Deep-Q network (with default parameters in rl_curiosity.py).

### Formalization

- States: Continuous states: Two first Euler angles.
- Actions: 9 discrete actions: Speed (0, +/-15) & Steer (0, +/-45)
- Reward function: Compute euclidean distance between instantaneous state and arbitrary goal state ([0.25, 0.5]), and take the opposite as reward function. Instantaneous state equalling goal state will correspond to maximum reward.

### Results

Average reward tends to -0.01 after t=20000.
This correspond to the ball getting close, on average, to the goal posture, after a few minutes.
This result seemed to be consistent over five trials with different random initial states.