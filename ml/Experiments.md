# Experiments

## 1st experiment: Getting close to target position

* Input data: Direction to target (discretised)
* Actions: 3 bins
* Reward: Reward difference between current and previous distance to target.
* Curiosity: OFF
* Learning rate: 0.1
* Gamma: 0.3
* Epsilon: 0.1
* Model: Q-tables 5x5x5
* Policy: E-Greedy

```
python rl_curiosity.py --x-min 0.25 --x-max 2.5 --y-min 0. --y-max 4. --n-action-bins 3 --use-direction-to-target --reward-delta-dist-1 --curiosity-weight 0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 2 --time-balance 1 --learning-rate 0.1 --gamma 0.3 --epsilon 0.1
```

Results:
* Learning takes various durations due to the relatively high number of actions.
* The robot seems to come close to the target, yet continues to explore around it, without an end. As a result, it may not be perceived as learning something.
* This is normal: indeed, the reward function only rewards actions that reduces distance between robot and target. As such, the robot's goal is to come closer to the target, not to stabilise close to it.

## 2nd experiment: Getting and staying close to target

* Input data: Polar coordinates to target (discretised)
* Actions: Specific actions: forward, backward, oscillating left, oscillating right
* Reward: Reward difference between current and previous distance to target (with target radius, and specific values for stabilised position).
* Curiosity: OFF
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Model: Q-tables 4x4
* Policy: Boltzmann

```
python rl_curiosity.py --x-min 0.25 --x-max 2.5 --y-min 0. --y-max 4. --use-spec-actions --n-action-bins 2 --use-polar-coordinates-to-target --reward-delta-dist-3 --curiosity-weight 0 --model tables --n-state-tiles 4 --send-port 7765 --receive-port 7767 --use-robot --time-step 2 --time-balance 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 --policy "boltzmann" --use-tag-target --target-radius 0.2
```

Results:
* Learning typically takes between five and ten minutes for the robot to come close to the target and stabilise close to it.
* This experiment may engender different behaviours across learning: moving randomly, reflecting on which action to take, making mistakes, getting excited as it is coming closer to target, adjusting position close to target, making small oscillations close to target.
* By design, learning does not depend on the target's position.

## 3rd experiment: Reward silence

* Input data: Polar coordinates to target (discretised)
* Actions: 3 bins
* Reward: Reward silence
* Curiosity: OFF
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Model: Q-tables 4x4x4
* Policy: Boltzmann

```
python rl_curiosity.py --x-min 0.25 --x-max 2.5 --y-min 0. --y-max 4. --n-action-bins 3 --use-constant-state --reward-sound-level --curiosity-weight 0 --model tables --n-state-tiles 4 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 --policy "boltzmann"
```

Results:
* Learning is very rapid: the robot stays still to stop making noises, typically after one minute.
* If one knows that the robot has hearing abilities, the resulting behaviour is quite intriguing: after a short time making agitated moves and loud noises, the robot suddenly stops moving and becomes silent. This pushed me to remain very silent in order not to disrupt the robot.
* By extension, silence could be used as a communication modality between an audience and the robot. This could create an intimately empathetic experience for the audience, staying silent and still together with the robot, as in a joint meditation.