import argparse
import sys
import time
import math

import pandas
import numpy as np
from keras.models import load_model
from sklearn.preprocessing import MinMaxScaler

from keras.models import Sequential
from keras.layers import Dense, InputLayer
from keras.utils.np_utils import to_categorical

from pythonosc import dispatcher
from pythonosc import osc_server

from pythonosc import osc_message_builder
from pythonosc import udp_client

# This is to allow the inclusion of common python code in the ../mp folder
import sys
sys.path.append('../')

import mp.preprocessing as mpp

# Returns the signed difference between two angles.
def dist_angles(a1, a2):
    return math.atan2(math.sin(a1-a2), math.cos(a1-a2))

# Returns the difference between current and previous datapoints.
def delta(data, prev_data):
    two_pi = 2*math.pi
    d = data - prev_data
    d[6] = dist_angles(data[6]*two_pi, prev_data[6]*two_pi)
    d[7] = dist_angles(data[7]*math.pi, prev_data[7]*math.pi)
    d[8] = dist_angles(data[8]*two_pi, prev_data[8]*two_pi)
    return d

# Returns the state from complete datapoint (including both data and deltas).
def get_state(complete_data, columns):
    return np.reshape(complete_data[columns], (1, len(columns)))

# Extrinsic reward function helpers.
def reward_sum(complete_data, columns, absolute=True, invert=False):
    complete_data = complete_data[ columns ]
    if absolute:
        complete_data = abs(complete_data)
    r = sum(complete_data) / len(complete_data)
    if invert:
        r = 1 - r
    return r

def reward_position_center(complete_data):
    return reward_sum(complete_data-0.5, [0, 1], absolute=True, invert=True)

def reward_inv_position_border(complete_data):
    complete_data = (complete_data - 0.5) * 2 # remap to [-1, +1]
    x = complete_data[0]
    y = complete_data[1]
    dist = np.sqrt(x*x + y*y) # get the distance from center
    if dist > 0.5:
        return -10
    else:
        return 0

def reward_delta_euler(complete_data):
    return reward_sum(complete_data, [15, 16, 17], absolute=True, invert=False)

def reward_inv_delta_euler(complete_data):
    return reward_sum(complete_data, [15, 16, 17], absolute=True, invert=True)

def reward_euler_state(complete_data):
    goal_euler_state = [0.25, 0.5, 0.5]
    dist = abs(complete_data[6] - goal_euler_state[0])**2
    dist += abs(complete_data[7] - goal_euler_state[1])**2
    #dist += abs(complete_data[8] - goal_euler_state[2])
    dist = -dist
    return dist

def reward(complete_data, reward_functions):
    n_functions = len(reward_functions)
    if n_functions == 0:
        return 0
    else:
        r = 0
        for f in reward_functions:
            r += f(complete_data)
        r /= n_functions
        return r

def choose_action_argmax(prediction):
    return np.argmax(prediction)

def choose_action_random(n_actions):
    return np.random.randint(n_actions)

def choose_action_softmax(prediction, temperature=1):
    # Source: https://gist.github.com/alceufc/f3fd0cd7d9efb120195c
    if (temperature != 1):
        prediction = np.power(prediction, 1. / temperature)
        prediction /= prediction.sum(0)
    return np.asscalar(np.random.choice(np.arange(len(prediction)), 1, p=prediction))

if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()

    parser.add_argument("-nq", "--n-hidden-q", type=int, default=64, help="Number of hidden units per layer for the Q-function")
    parser.add_argument("-nf", "--n-hidden-forward", type=int, default=64, help="Number of hidden units per layer for the forward function")

    parser.add_argument("-p", "--policy", type=str, default="greedy", choices=["greedy", "boltzmann", "mixed"], help="Agent policy")
    parser.add_argument("-eps", "--epsilon", type=float, default=0.1, help="Epsilon value for the 'greedy' policy")
    parser.add_argument("-temp", "--temperature", type=float, default=1, help="Temperature value for the 'boltzmann' policy [0, +inf] (higher: more uniform, lower: more greedy")

    parser.add_argument("-gam", "--gamma", type=float, default=0.95, help="Gamma value for the Q-learning")

    parser.add_argument("-cw", "--curiosity-weight", type=float, default=0.5, help="Weight of curiosity intrinsic reward (as %%)")

    parser.add_argument("-t", "--time-step", type=float, default=0, help="Period (in seconds) of each step (0 = as fast as possible)")

    parser.add_argument("--use-position", default=False, action='store_true', help="Add position to the input data")
    parser.add_argument("--use-delta-position", default=False, action='store_true', help="Add delta position to the input data")
    parser.add_argument("--use-quaternion", default=False, action='store_true', help="Add quaternion to the input data")
    parser.add_argument("--use-delta-quaternion", default=False, action='store_true', help="Add delta quaternion to the input data")
    parser.add_argument("--use-euler", default=False, action='store_true', help="Add Euler angles to the input data")
    parser.add_argument("--use-euler-2", default=False, action='store_true', help="Add Euler angles to the input data")
    parser.add_argument("--use-delta-euler", default=False, action='store_true', help="Add delta Euler angles to the input data")

    parser.add_argument("--reward-position-center", default=False, action='store_true', help="Reward being close to center (0, 0)")
    parser.add_argument("--reward-inv-position-border", default=False, action='store_true', help="Punish heavily being too close to the edges")
    parser.add_argument("--reward-delta-euler", default=False, action='store_true', help="Reward Euler motion")
    parser.add_argument("--reward-inv-delta-euler", default=False, action='store_true', help="Reward no Euler motion")
    parser.add_argument("--reward-euler-state", default=False, action='store_true', help="Reward static Euler state")

    parser.add_argument("--n-bins", type=int, default=3,
                        help="Number of bins to use for classification (only valid if used with --classification)")

#    parser.add_argument("-D", "--model-directory", type=str, default=".", help="The directory where to save models")
#    parser.add_argument("-P", "--prefix", type=str, default="ann-regression-", help="Prefix to use for saving files")

    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to send data to.")
    parser.add_argument("-s", "--send-port", default="8765",
                        type=int, help="Specify the port number to listen on.")
    parser.add_argument("-r", "--receive-port", default="8767",
                        type=int, help="Specify the port number to listen on.")

    # Parse arguments.
    args = parser.parse_args()

    notify_recv = False
    perf_measurements = []
    rows = 0

    n_bins = args.n_bins
    n_actions = n_bins*n_bins

    time_step = args.time_step

    policy = args.policy

    epsilon = np.clip(args.epsilon, 0, 1)
    temperature = np.max(args.temperature, 0)

    gamma = np.clip(args.gamma, 0, 1)
    curiosity_weight = np.clip(args.curiosity_weight, 0, 1)
    print('curiosity_weight ', curiosity_weight)

    # Build filtering columns and n_inputs.
    state_columns = []
    if args.use_position:
        state_columns += [0, 1]
    if args.use_quaternion:
        state_columns += [2, 3, 4, 5]
    if args.use_euler:
        state_columns += [6, 7, 8]
    if args.use_euler_2:
        state_columns += [6, 7]
    if args.use_delta_position:
        state_columns += [9, 10]
    if args.use_delta_quaternion:
        state_columns += [11, 12, 13, 14]
    if args.use_delta_euler:
        state_columns += [15, 16, 17]
    n_inputs = len(state_columns)
    if n_inputs <= 0:
        exit("You have no inputs! Make sure to specify some inputs using the --use-* options.")

    # Build array of extrinsic rewards.
    extrinsic_reward_functions = []
    if args.reward_position_center:
        extrinsic_reward_functions += [reward_position_center]
    if args.reward_inv_position_border:
        extrinsic_reward_functions += [reward_inv_position_border]
    if args.reward_delta_euler:
        extrinsic_reward_functions += [reward_delta_euler]
    if args.reward_inv_delta_euler:
        extrinsic_reward_functions += [reward_inv_delta_euler]
    if args.reward_euler_state:
        extrinsic_reward_functions += [reward_euler_state]

    # Initialize stuff.
    n_inputs_q = n_inputs
    n_inputs_forward = n_inputs + n_actions

    n_hidden_q = args.n_hidden_q
    n_hidden_forward = args.n_hidden_forward

    # Compile model Q(state_t, action_t)
    model_q = Sequential()
    model_q.add(InputLayer(batch_input_shape=(1, n_inputs_q)))
    model_q.add(Dense(n_hidden_q, activation='relu'))
    model_q.add(Dense(n_actions, activation='softmax'))
    model_q.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
    print(model_q.summary())

    # Predicts state_{t+1} = f(state_t, action_t)
    model_forward = Sequential()
    model_forward.add(InputLayer(batch_input_shape=(1, n_inputs_forward)))
    model_forward.add(Dense(n_hidden_forward, activation='relu'))
    model_forward.add(Dense(n_inputs_q, activation='linear'))
    model_forward.compile(loss='mse', optimizer='adam', metrics=['mae'])
    print(model_forward.summary())

    # Initialize stuff.
    prev_data = None
    prev_time = -1
    prev_state = None
    prev_action = -1
    avg_r = None
    avg_r = np.array([ 0., 0., 0. ])

    # Create rescalers.
    scalerX = MinMaxScaler()
    scalerX.fit([[-25, -25, -1, -1, -1, -1, -180, -90, -180],
                 [+25, +25, +1, +1, +1, +1, +180, +90, +180]])

    scalerY = MinMaxScaler()
    scalerY.fit([[-15, -45],
                 [+15, +45]])

    iter = 0
    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed, steer):
        global notify_recv
        # global model_q
        # global perf_measurements
        global prev_data, prev_time, prev_state, prev_action
        global avg_r, iter
        # global state_columns

        start_time = time.perf_counter()

        if not(notify_recv):
            print("Received initial packets from unity!")
            notify_recv = True

        # Process input data.
        pos = np.array([x, y])
        quat = np.array([qx, qy, qz, qw])
        euler = np.array(mpp.quaternion_to_euler(qx, qy, qz, qw))
        data = np.concatenate((pos, quat, euler))
        data = mpp.standardize(data, scalerX)[0] # normalize

        # If this is the first time we receive something: save as initial values and skip
        if prev_data is None:
            # Reset.
            prev_data = data
            prev_time = t
            delta_data = delta(data, data) # ie. 0
            complete_data = np.concatenate((data, delta_data))
            state = get_state(complete_data, state_columns)
            prev_state = state
            prev_action = 0 # dummy

        # Else: one step of Q-learning loop.
        else:
            # Compute state.
            delta_data = delta(data, prev_data) / (t - prev_time)
            complete_data = np.concatenate((data, delta_data))
            state = get_state(complete_data, state_columns)
            #print('state', state)

            # Adjust state model.
            state_model_input = np.concatenate((prev_state[0], to_categorical(prev_action, n_actions)))
            state_model_input = np.reshape(state_model_input, (1, n_inputs_forward))
            model_forward.fit(state_model_input, state, epochs=1, verbose=0)
            #print('state_model_input', state_model_input)

            # Calculate intrinsic reward (curiosity).
            predicted_state = model_forward.predict(state_model_input)
            prediction_error = np.linalg.norm(state - predicted_state)

            # Intrinsic reward = curiosity.
            r_int = prediction_error

            # Extrinsic reward.
            r_ext = reward(complete_data, extrinsic_reward_functions)

            r = curiosity_weight * r_int + (1 - curiosity_weight) * r_ext

            r_array = np.array([ r_int, r_ext, r ])

            if avg_r is None:
                avg_r = r_array
            else:
                avg_r -= 0.01 * (avg_r - r_array)

            # Save previous data information.
            prev_data = data
            prev_time = t

            # Perform one step.
            target = r + gamma * np.max(model_q.predict(state))
            target_vec = model_q.predict(prev_state)[0]
            target_vec[prev_action] = target
            model_q.fit(prev_state, target_vec.reshape(-1, n_actions), epochs=1, verbose=0)

        # Get prediction table.
        prediction = model_q.predict(state, verbose=0).squeeze()

        # Choose action.
        if policy == "greedy":
            if np.random.random() < epsilon:
                action = choose_action_random(n_actions)
            else:
                action = choose_action_argmax(prediction)

        elif policy == "boltzmann":
            action = choose_action_softmax(prediction, temperature)

        elif policy == "mixed":
            if np.random.random() < epsilon:
                action = choose_action_random(n_actions)
            else:
                action = choose_action_softmax(prediction, temperature)

        # Save action for next iteration.
        prev_action = action

        target = [mpp.class_to_speed_steering(action, n_bins, True)]
        #target = [[0., 0.5]]
       # print('target ', target)
#        print("Choice made {} {} {}".format(action, model.predict(state), target))

        # Send OSC message of action.
        action = scalerY.inverse_transform(target).astype('float')
        #print('send action', action)
#        print("Target {} Action {}".format(target, action))
        if iter % 100 == 0:
            print('--- + 100')
            print("t={} average reward = (int: {} ext: {} total: {})".format(iter, avg_r[0], avg_r[1], avg_r[2]))
            print("state = ", state)

        # Send OSC message.
        client.send_message("/morphoses/action", action[0])

        # Update state.
        prev_state = state

        iter += 1

        # Wait
        if time_step > 0:
            time.sleep(time_step)

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server & client.

    server = osc_server.BlockingOSCUDPServer(("0.0.0.0", args.receive_port), dispatcher)
    client = udp_client.SimpleUDPClient(args.ip, args.send_port)

    print(f"Serving on {server.server_address}. Program ready.")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print(f"Exiting program... {np.mean(perf_measurements)}")
        server.server_close()
        sys.exit()
