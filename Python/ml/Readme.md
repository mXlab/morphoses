# Experiments

Here, I report experiments made with the ball and specific RL formalizations/algorithms/parameterizations, to be able to reproduce them later.

# 1st experiment

### Goal behaviour

Stabilize into a static angular posture.

### Algorithm

Deep-Q network (with default parameters in rl_curiosity.py).

### Formalization

- States: Continuous states: Two first Euler angles (--use-euler-firsts)
- Actions: 9 discrete actions: Speed (0, +/-15) & Steer (0, +/-45)
- Reward function: Compute norm between instantaneous state and arbitrary goal state ([0.25, 0.5]), and take the opposite as reward function. Instantaneous state equalling goal state will correspond to maximum reward. (--reward-euler-state-1)
- Curiosity: Only extrinsic reward (--curiosity-weight 0.).

### Results

Average reward tends to -0.01 after t=20000. This correspond to the ball getting close, on average, to the goal posture, after a few minutes.
This result seemed to be consistent over five trials with different random initial states. A next step would be to better understand the step-by-step learning process of the ball, possibly by focusing on the kinematics of the ball.

# 2nd experiment

### Goal behaviour

Stabilize into a static angular posture.

### Algorithm

Deep-Q network (with default parameters in rl_curiosity.py).

### Formalization

I modified the BallController.cs script to give the ball a certain kinetic sense.
Precisely, I turned the previous "speed" action (having a dynamic, non-reproducible physical consequence over the ball's angular state) into a "position" action (having a static, reproducible consequence over the ball's angular state). Now, each action from a given state will always trigger the same new state.

To inject our knowledge of the physical properties of the environment to the ball, I dropped reinforcement learning time step to 0.5 seconds (--time-step .5). The idea is that it would allow the ball to learn better, even if it from less training data. Precisely, waiting for a certain time before rewarding the ball from its action lets the ball stabilize into a static angular state. This helps teach the ball a sense of reproductibility of its actions toward its angular state.

- States: Continuous states: First and third Euler angles (corresponding to those modified by "speed" and "steer" actions) (--use-euler-extremes)
- Actions: 9 discrete actions: Frontward/static/backward step (0, +/-15) & Left/static/right step (0, +/-45) (modified BallController.cs script)
- Reward function: Compute euclidean distance between instantaneous state and goal state, corresponding to arbitrary first Euler angle and zero steering ([0.25, 0.75]). If distance > 0.1, then return -10. Else, return 0. (--reward-euler-state-2)
- Curiosity: Only extrinsic reward (--curiosity-weight 0.).

### Results

The ball gets close to the goal state quite quickly (even with a 0.5 second time step, and less training data). Yet, it does not exactly reach the goal state (because of the tolerated radius around the goal state).
Interestingly, the ball seems to converge toward the (0., 0.) action, which visually correponds to a still ball.
A next goal would be to fine-tune the reward function so that it gets a better sense of closeness toward the goal state.

# 3rd experiment

### Goal behaviour

Stabilize into a static angular posture.

### Algorithm

Deep-Q network (with default parameters in rl_curiosity.py).

### Formalization

I kept the same modifications in the BallController.cs script to give the ball a certain kinetic sense.

I dropped reinforcement learning time step to 0.5 seconds (--time-step .5).

- States: Continuous states: First and third Euler angles (corresponding to those modified by "speed" and "steer" actions) (--use-euler-extremes)
- Actions: 9 discrete actions: Frontward/static/backward step (0, +/-15) & Left/static/right step (0, +/-45) (modified BallController.cs script)
- Reward function: I give a huge penalty to the ball (-100.) any time it steers left or right (i.e., if distance between instantaneous third angular state and third angular goal state > 0.025). In the case where the ball is at zero steering, I give a penalty that is proportional to the norm between instantaneous first angular state and first angular goal state, corresponding to arbitrary first Euler angle (0.25). (--reward-euler-state-3)
- Curiosity: Only extrinsic reward (--curiosity-weight 0.).

### Results

The ball gets very close ([0.23110006 0.75000171]) to the goal state ([0.25 0.75]) after t=330.
Interestingly, the ball seems to converge toward the (0., 0.) action, which visually correponds to a still ball. Sometimes, the ball explores an action, but the huge penalty toward the steering makes the ball expressively move back to its previous zero steering state.
A next goal could be to try different action formalizations, as well as time step implementations, to witness different expressive ball behaviours around its angular goal state. One could also investigate intrinsic reward strategies to better control the exploration behaviour of the ball.

# 4th experiment

### Goal behaviour

Move toward the center of the environment.

### Algorithm

Deep-Q network (with default parameters in rl_curiosity.py).

### Formalization

I took the two position coordinates plus the three Euler angles as state representation. This makes a quite big state space, but here, we cannot only take the position as input state. Indeed, the ball has to know more than only its position in the plane to learn to take good actions. For example, for a given position in the plane, the same action could lead the ball to a completely different state, depending on the initial orientation of the ball.

I dropped reinforcement learning time step to 0.5 seconds (--time-step .5).

- States: Continuous states: Two position coordinates in the plane + Three Euler angles (--use-position --use-euler)
- Actions: 9 discrete actions: Speed (0, +/-15) & Steer (0, +/-45)
- Reward function: I give a huge penalty to the ball (-100.\*dist) if dist between instantaneous position and goal position (center of the map) > 0.25. I give a least huge penalty to the bal (-10.\*dist) else. (--reward-position-state)
- Curiosity: Only extrinsic reward (--curiosity-weight 0.).

### Results

As expected, the ball has more difficulties to learn to get close to the center (still unstable at t=2000). My opinion is that the state space is now really big to learn something in a short amount of time. Pre-training could be an option, or letting the ball learn during more time. Another option would be to engineer actions that would reduce the state space explored (for example, forcing the ball to move inside a grid).
