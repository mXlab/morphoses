import numpy as np
import state, reward, action

from keras.models import Sequential
from keras.layers import Dense, InputLayer
from keras.utils.np_utils import to_categorical
from keras import optimizers

import time
import tilecoding.representation as rep

class Agent:
    def __init__(self, name, world, **kwargs):
        self.name = name
        self.world = world

        self.policy = kwargs.get('policy', 'greedy')
        self.epsilon = np.clip(kwargs.get('epsilon', 0.1), 0, 1)
        self.temperature = np.max(kwargs.get('temperature', 1), 0)
        self.gamma = np.clip(kwargs.get('gamma', 0.95), 0, 1)
        self.learning_rate = np.max(kwargs.get('learning_rate', 0.01), 0)
        self.curiosity_weight = np.clip(kwargs.get('curiosity_weight', 0.5), 0, 1)

        self.time_step = np.max(kwargs.get('time_step', 0), 0)
        self.time_balance = np.max(kwargs.get('time_balance', 0), 0)

        self.use_sarsa = kwargs.get('use_sarsa', True)

        self.state_profile = kwargs.get('state_profile', [])
        self.n_inputs = len(self.state_profile)
        if self.n_inputs <= 0:
            raise RuntimeError("You have no inputs! Make sure to specify some inputs.")

        reward_profile = kwargs.get('reward_profile', [])
        self.extrinsic_rewards = get_extrinsic_rewards(reward_profile)

        action_profile = kwargs.get('action_profile', 'grid')
        self.action_set = action.create_action_set(action_profile)
        self.n_actions = self.action_set.n_actions()

        # Build Q-function model.
        q_model_type = kwargs.get('q_model_type', 'tables')
        self.use_ann = not q_model_type == "tables"
        self.use_tile_coding = not q_model_type == "ann"

        # Create tiling if needed.
        if self.use_tile_coding:
            n_state_tiles = kwargs.get('n_state_tiles', 10)
            n_state_tilings = kwargs.get('n_state_tilings', 1)
            self.tile_coding = rep.TileCoding(input_indices=[np.arange(self.n_inputs)],
                                         ntiles=[n_state_tiles],
                                         ntilings=[n_state_tilings],
                                         hashing=None,
                                         bias_term=False,
                                         state_range=[np.full(self.n_inputs, 0), np.full(self.n_inputs, 1)],
                                         rnd_stream=np.random.RandomState())
            self.n_inputs_q = self.tile_coding.size
        else:
            self.tile_coding = None
            self.n_inputs_q = self.n_inputs

        # Create model Q(state_t, action_t).
        n_hidden_q = kwargs.get('n_hidden_q', 64)
        if self.use_ann:
            self.model_q = Sequential()
            self.model_q.add(InputLayer(batch_input_shape=(1, self.n_inputs_q)))
            if n_hidden_q > 0:
                self.model_q.add(Dense(n_hidden_q, activation='relu'))
            self.model_q.add(Dense(self.n_actions, activation='linear'))
            # model_q.add(Dense(n_actions, activation='softmax'))
            self.model_q.compile(loss='categorical_crossentropy',
                                 optimizer=optimizers.SGD(lr=self.learning_rate),
                                 metrics=['accuracy'])
            print("=== Model Q ===")
            print(self.model_q.summary())
        else:
            # Table.
            self.model_q = np.zeros((self.n_inputs_q, self.n_actions))

        # Create forward model (for curiosity): predicts state_{t+1} = f(state_t, action_t)
        self.n_inputs_forward = self.n_inputs + self.n_actions
        n_hidden_forward = kwargs.get('n_hidden_forward', 64)
        self.model_forward = Sequential()
        self.model_forward.add(InputLayer(batch_input_shape=(1, self.n_inputs_forward)))
        if n_hidden_forward > 0:
            self.model_forward.add(Dense(n_hidden_forward, activation='relu'))
        self.model_forward.add(Dense(self.n_inputs, activation='linear'))
        self.model_forward.compile(loss='mse', optimizer='adam', metrics=['mae'])
        print("=== Model forward ===")
        print(self.model_forward.summary())

    def get_name(self):
        return self.name

    def begin(self):
        self.prev_state = self.get_state()
        self.prev_action = 0 # dummy
        self.r = 0
        self.avg_r = None
        self.max_r = -9999
        self.min_r = +9999
        self.iter = 0

    def step(self):
        # If the robot is within virtual fence: Perform standard RL loop.
#        if self.is_inside_boudaries():
        if True:
            self.step_rl()
#        else:
#            self.step_recenter()

    # Performs one step of Q-learning loop.
    def step_rl(self):

        # Get current state.
        state = self.get_state()

        # Adjust state model.
        state_model_input = np.concatenate((self.prev_state[0], to_categorical(self.prev_action, self.n_actions)))
        state_model_input = np.reshape(state_model_input, (1, self.n_inputs_forward))

        self.model_forward.fit(state_model_input, state, epochs=1, verbose=0)

        # Calculate intrinsic reward (curiosity).
        predicted_state = self.model_forward.predict(state_model_input)
        prediction_error = np.linalg.norm(state - predicted_state)

        # Intrinsic reward ie. curiosity.
        r_int = prediction_error

        # Extrinsic reward.
        r_ext = reward.reward(self.world, self, self.extrinsic_rewards)

        # Compute total reward (intrinsic and extrinsic).
        r = self.curiosity_weight * r_int + (1 - self.curiosity_weight) * r_ext
        self.min_r = min(self.min_r, r)
        self.max_r = max(self.max_r, r)
        r = np.clip(r, self.min_r, self.max_r)
        scaled_r = (r - self.min_r) / (self.max_r - self.min_r + 0.000001) # TODO: improve this
        brightness = round(scaled_r * 255)
        self.world.set_color(self, [255 - brightness, brightness, 0])

#            print(r)

        r_array = np.array([ r_int, r_ext, r ])

        if self.avg_r is None:
            self.avg_r = r_array
        else:
            self.avg_r -= (1-self.gamma) * (self.avg_r - r_array)



        print("{} => {}".format(state, r))
        n_iter_log = 10
        if self.iter % n_iter_log == 0:
            print("t={} average reward = (int: {} ext: {} total: {})".format(iter, self.avg_r[0], self.avg_r[1], self.avg_r[2]))
            print("state = ", state)
#            print("counts = ", count_action / sum(count_action))
            self.avg_r = r_array # reset
            print("MODEL: ")
            print(self.model_q)


        if self.use_ann:
            prediction = self.model_q.predict(state_to_tile_coding(state, self.tile_coding), verbose=0).squeeze()
        else:
            prediction = q_table_predict(self.model_q, state, self.tile_coding)

        # Choose action.
        if self.policy == "greedy":
            if np.random.random() < self.epsilon:
                action = choose_action_random(self.n_actions)
            else:
                action = choose_action_argmax(prediction)

        elif self.policy == "boltzmann":
            action = choose_action_softmax(prediction, self.temperature)

        elif self.policy == "mixed":
            if np.random.random() < self.epsilon:
                action = choose_action_random(self.n_actions)
            else:
                action = choose_action_softmax(prediction, self.temperature)

        #count_action[action] += 1

        # Perform one step.
        if self.use_sarsa:
            target = r + self.gamma * prediction[action]
        else:
            target = r + self.gamma * np.max(prediction)

        # Perform one step.
        # Source: https://keon.io/deep-q-learning/
        # learned value = r + gamma * max_a Q(s_{t+1}, a)
        if self.use_ann:
            target_vec = self.model_q.predict(state_to_tile_coding(self.prev_state, self.tile_coding))[0] # Q(s_t, a_t)
            target_vec[self.prev_action] = target
            self.model_q.fit(state_to_tile_coding(self.prev_state, self.tile_coding), target_vec.reshape(-1, n_actions), epochs=1, verbose=0)
        else:
            target_vec = q_table_predict(self.model_q, self.prev_state, self.tile_coding)
            q_table_update(self.model_q, state, self.tile_coding, target, self.prev_action, self.learning_rate)


        # Convert action ID to actual action.
        actual_action = self.action_set.get_action(action)

        # Perform action in world.
        self.world.do_action(self, actual_action)

        # Save action and state for next iteration.
        self.prev_action = action
        self.prev_state = state

        self.iter += 1

        # Wait
        if self.time_step > 0:
            self.world.sleep(self.time_step)

        # Structure learning by having the robot take a pause in a stabilised state (zero speed, zero steering).
        if self.time_balance > 0:
            self.world.do_action(self.name(), [0, 0])
            self.world.sleep(self.time_balance)

    def step_recenter(self):
        pass

    def get_state(self):
        return np.reshape(np.array(self.world.get(self, self.state_profile)), (1, self.n_inputs))


# Return list of reward functions from textual reward profile.
def get_extrinsic_rewards(reward_profile):
    extrinsic_rewards = []
    for reward_name in reward_profile:
        reward_function = getattr(reward, "reward_" + reward_name)
        extrinsic_rewards += [ reward_function ]
    return extrinsic_rewards

# State to tile coding - one hot encoding
def state_to_tile_coding(state, tc):
    if (tc != None):
        one_hot = np.zeros(tc.size)
        code = tc(state[0])
        if code.size == 1:
            one_hot_value = 1
        else:
            one_hot_value = 1/len(code)
        one_hot[code] = one_hot_value
        return np.reshape(one_hot, (1, tc.size))
    else:
        return state

# Returns an array of estimated Q values for state "state" for each action.
def q_table_predict(q_table, state, tc):
    code = tc(state[0])
    if code.size == 1:
        code = [code]
    q_sum = np.zeros(q_table.shape[1])
    for c in code:
        q_sum += q_table[c]
    return q_sum

def q_table_update(q_table, state, tc, target, action, lr):
    code = tc(state[0])
    if code.size == 1:
        code = [code]
    n_bins = len(code)
    x = 0
    for c in code:
        # print("Update Q({},{}): {} to target {} with lr {}".format(c, action, q_table[c][action], target / n_bins, lr))
        q_table[c,action] -= lr * (q_table[c][action] - target / n_bins)
        # print("==> {}".format(q_table[c][action]))


def choose_action_argmax(prediction):
    allmax = np.argwhere(prediction == np.amax(prediction)).flatten()
    return np.random.choice(allmax, 1).item()
#    return np.asscalar(np.random.choice(allmax, 1))
#    return np.argmax(prediction)

def choose_action_random(n_actions):
    return np.random.randint(n_actions)

def choose_action_softmax(prediction, temperature=1):
    # Source: https://gist.github.com/alceufc/f3fd0cd7d9efb120195c
    prediction = np.exp(prediction / temperature)
    # if (temperature != 1):
    #     prediction = np.power(prediction, 1. / temperature)
    # if (prediction.sum() == 0):
    #     prediction.fill(1)
    prediction /= prediction.sum()
    print("Prediction: {}".format(prediction))
    return np.random.choice(np.arange(len(prediction)), 1, p=prediction).item()
#    return np.asscalar(np.random.choice(np.arange(len(prediction)), 1, p=prediction))
