import argparse
import numpy as np
from gym.spaces import Box, Discrete

from keras.models import Sequential
from keras.layers import Dense, Activation, Flatten
from keras.optimizers import Adam

from rl.agents import SARSAAgent, DQNAgent
from rl.policy import BoltzmannQPolicy, LinearAnnealedPolicy, EpsGreedyQPolicy
from rl.memory import SequentialMemory

import sys
sys.path.append('../')

from mp.osc_env import OscEnv

# Create parser
parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument("n_actions", type=int, help="Number of different action")
parser.add_argument("n_observations", type=int, help="Number of different action")

parser.add_argument("--observation-min", type=float, default=0, help="Minimum value of observation")
parser.add_argument("--observation-max", type=float, default=0, help="Maximum value of observation")
#parser.add_argument("--observation-ip", default="127.0.0.1", help="Specify the ip address to listen on for observations and reward.")
parser.add_argument("--observation-port", default=8000, type=int, help="Specify the port number to listen on.")
parser.add_argument("--action-ip", default="127.0.0.1", help="Specify the ip address to send actions.")
parser.add_argument("--action-port", default=8001, type=int, help="Specify the port number to send.")
parser.add_argument("--observation-address", default="/env/observation", help="The OSC address to send observations.")
parser.add_argument("--action_address", default="/env/action", help="The OSC address to send actions.")

parser.add_argument("--use-sarsa", default=False, action='store_true', help="Use SARSA instead of Q-Learning")
parser.add_argument("--n-steps", type=int, default=50000, help="Max. number of steps")
parser.add_argument("--learning-rate", type=float, default=1e-3, help="The learning rate of the learning agent")

# Parse arguments.
args = parser.parse_args()

# Get the environment and extract the number of actions.
env = OscEnv(Discrete(args.n_actions), Box(low=args.observation_min, high=args.observation_max, shape=(args.n_observations,)),
                action_ip=args.action_ip, action_port=args.action_port, observation_port=args.observation_port,
                action_address=args.action_address, observation_address=args.observation_address)
# np.random.seed(123)
# env.seed(123)
nb_actions = env.action_space.n

# Next, we build a very simple model.
model = Sequential()
model.add(Flatten(input_shape=(1,) + env.observation_space.shape))
model.add(Dense(nb_actions))
model.add(Activation('linear'))
print(model.summary())

# Build the policy.
policy = BoltzmannQPolicy()
#policy = LinearAnnealedPolicy(EpsGreedyQPolicy(), attr='eps', value_max=1., value_min=.1, value_test=.05,
#                               nb_steps=10000)

if args.use_sarsa:
    # SARSA does not require a memory.
    agent = SARSAAgent(model=model, nb_actions=nb_actions, nb_steps_warmup=10, policy=policy)
else:
    memory = SequentialMemory(limit=50000, window_length=1)
    agent = DQNAgent(model=model, memory=memory, nb_actions=nb_actions, nb_steps_warmup=50, policy=policy)

agent.compile(Adam(lr=args.learning_rate), metrics=['mae'])

# Okay, now it's time to learn something! We visualize the training here for show, but this
# slows down training quite a lot. You can always safely abort the training prematurely using
# Ctrl + C.
agent.fit(env, nb_steps=args.n_steps, visualize=False, verbose=2)

# After training is done, we save the final weights.
#sarsa.save_weights('sarsa_osc_weights.h5f', overwrite=True)

# Finally, evaluate our algorithm for 5 episodes.
#sarsa.test(env, nb_episodes=5, visualize=True)
