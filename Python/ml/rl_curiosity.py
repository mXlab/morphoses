import argparse
import sys
import time
import math
import signal
import random

import pandas
import numpy as np
import matplotlib.pyplot as plt
from keras.models import load_model
from sklearn.preprocessing import MinMaxScaler

from keras.models import Sequential
from keras.layers import Dense, InputLayer
from keras.utils.np_utils import to_categorical
from keras import optimizers

from pythonosc import dispatcher
from pythonosc import osc_server

from pythonosc import osc_message_builder
from pythonosc import udp_client

# This is to allow the inclusion of common python code in the ../mp folder
import sys
sys.path.append('../')

import mp.preprocessing as mpp
import tilecoding.representation as rep

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

# Deprecated: Returns the robot's instantaneous orientation.
def compute_orientation(delta_data, speed):
    forward = np.sign(speed)
    return np.angle(np.complex(forward*delta_data[0], forward*delta_data[1]))

# Returns the robot's instantaneous direction to target, discretised as 0=front, 1=right, 2=left, 3=back. Optional: Roughly plots relevant vectors and past robot positions.
def compute_direction_to_target(data, delta_data, target_position_state, speed, plot):
    print('pos', [data[0], data[1]])

    # Computer vector for robot instantaneous direction.
    forward = np.sign(speed)
    delta_pos = [forward*delta_data[0], forward*delta_data[1]] # invert according to current speed
    print("velocity: ({}, {}) speed: {} orientation: {}".format(delta_pos[0], delta_pos[1], speed, np.degrees(compute_orientation(delta_data, speed))))

    # Compute vector from robot to target state.
    delta_robot_to_target = [target_position_state[0] - data[0], target_position_state[1] - data[1]]

    # Normalize vectors.

    delta_pos = delta_pos / np.linalg.norm(delta_pos)
    delta_robot_to_target = delta_robot_to_target / np.linalg.norm(delta_robot_to_target)

    # Compute relative angle.
    dot_product = np.dot(delta_pos, delta_robot_to_target)
    angle = np.arccos(dot_product)
    angle = np.rad2deg(angle)

    # If target in a 90-degree angular extent in front of the robot: front.
    if -45. < angle < 45:
        state = 0.

    # If target in a 90-degree angular extent at right of the robot: right.
    elif -135. < angle < -45:
        state = 1. / 3.

    # If target in a 90-degree angular extent at left of the robot: left.
    elif 45 < angle < 135:
        state = 2. / 3.

    # If target in a 90-degree angular extent behind the robot: back.
    else:
        state = 1.

    # Optional: Roughly plots relevant vectors and past robot positions.
    if plot:
        origin = [data[0]], [data[1]]

        plt.subplot(121)
        plt.cla()
        plt.xlim(0., 1.)
        plt.ylim(0., 1.)
        plt.quiver(*origin, [delta_pos[0], delta_robot_to_target[0]], [delta_pos[1], delta_robot_to_target[1]], color=['r','b'])
        plt.scatter(target_position_state[0], target_position_state[1])
        plt.xlabel('state: ' + str(state) + ', ' + 'angle: ' + str(angle))

        plt.subplot(122)
        plt.xlim(0., 1.)
        plt.ylim(0., 1.)
        plt.scatter(data[0], data[1], s=1, c='#000000')
        #plt.scatter(target_position_state[0], target_position_state[1])
        plt.draw()    
        plt.pause(0.001)

    return state

# Returns the robot's instantaneous distance to target relative to a target radius, discretised as 0=outside neighbourhood, 1=inside neighbourhood
def compute_distance_to_target(data, target_position_state, target_radius):
    
    # Compute current distance from target position state.
    dist = np.sqrt(abs(data[0] - target_position_state[0])**2 + abs(data[1] - target_position_state[1])**2)

    # If robot far away from target: outside neighbourhood.
    if dist > target_radius:
        state = 0.

    # If robot far away from target: inside neighbourhood.
    else:
        state = 1.

    return state

# Returns the state from complete datapoint (including both data and deltas).
def get_state(complete_data, columns):
    return np.reshape(complete_data[columns], (1, len(columns)))

# State to tile coding - one hot encoding
def state_to_tile_coding(state, tc):
    if (tc != None):
        one_hot = np.zeros(tc.size)
        code = tc(state[0])
        if isinstance(code, list):
            one_hot_value = 1/len(code)
        else:
            one_hot_value = 1
        one_hot[code] = one_hot_value
        return np.reshape(one_hot, (1, tc.size))
    else:
        return state

# Returns an array of estimated Q values for state "state" for each action.
def q_table_predict(q_table, state, tc):
    code = tc(state[0])
    if not isinstance(code, list):
        code = [code]
    q_sum = np.zeros(q_table.shape[1])
    for c in code:
        q_sum += q_table[c]
    return q_sum

def q_table_update(q_table, state, tc, target, action, lr):
    code = tc(state[0])
    if not isinstance(code, list):
        code = [code]
    n_bins = len(code)
    x = 0
    for c in code:
        # print("Update Q({},{}): {} to target {} with lr {}".format(c, action, q_table[c][action], target / n_bins, lr))
        q_table[c,action] -= lr * (q_table[c][action] - target / n_bins)
        # print("==> {}".format(q_table[c][action]))

# Extrinsic reward function helpers.
def reward_sum(complete_data, columns, absolute=True, invert=False):
    complete_data = complete_data[ columns ]
    if absolute:
        complete_data = abs(complete_data)
    r = sum(complete_data) / len(complete_data)
    if invert:
        r = -r
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

def reward_delta_roll(complete_data):
    return reward_sum(complete_data, [16], absolute=True, invert=False)

def reward_inv_delta_roll(complete_data):
    return reward_sum(complete_data, [16], absolute=True, invert=True)

def reward_delta_pitch(complete_data):
    return reward_sum(complete_data, [17], absolute=True, invert=False)

def reward_inv_delta_pitch(complete_data):
    return reward_sum(complete_data, [17], absolute=True, invert=True)

def reward_delta_yaw(complete_data):
    return reward_sum(complete_data, [18], absolute=True, invert=False)

def reward_inv_delta_yaw(complete_data):
    return reward_sum(complete_data, [18], absolute=True, invert=True)

def reward_delta_euler(complete_data):
    return reward_sum(complete_data, [16, 17, 18], absolute=True, invert=False)

def reward_inv_delta_euler(complete_data):
    return reward_sum(complete_data, [16, 17, 18], absolute=True, invert=True)


def reward_euler_state_1(complete_data):
    target_euler_state = [0.25, 0.5, 0.75] # do not care about third Euler angle

    dist = abs(complete_data[6] - target_euler_state[0])**2
    dist += abs(complete_data[7] - target_euler_state[1])**2
    dist = -dist
    return dist

def reward_euler_state_2(complete_data):
    target_euler_state = [0.25, 0.5, 0.75] # do not care about second Euler angle

    dist = abs(complete_data[6] - target_euler_state[0])**2
    dist += abs(complete_data[8] - target_euler_state[2])**2
    dist = np.sqrt(dist)
    if dist > 0.1:
        return -10.
    else:
        return 0.

def reward_euler_state_3(complete_data):
    target_euler_state = [0.25, 0.5, 0.75] # do not care about second Euler angle

    dist_1 = abs(complete_data[6] - target_euler_state[0])
    dist_3 = abs(complete_data[8] - target_euler_state[2])
    if dist_3 > 0.025:
        return -100.
    else:
        return -10. * dist_1

# Calculate distance between two angles normalized in [0, 1]
def normalized_angle_dist(x1, x2):
    return min( abs(x2-x1), abs(x2-x1+1))

def reward_euler_state_robot_1(complete_data):
    target_euler_state = [ 0.7138384425182009, 0.8537683086573894, 0.14679902767144734 ]
    # target_euler_state = [ 0.07599221,  0.11881516,  0.4418379 ]

    dist = normalized_angle_dist(complete_data[6], target_euler_state[0])**2
    dist += normalized_angle_dist(complete_data[7], target_euler_state[1])**2
    dist += normalized_angle_dist(complete_data[8], target_euler_state[2])**2
    dist = 1 / (dist + 1)
    return dist

def reward_position_state(complete_data):
    global target_position_state

    dist = abs(complete_data[0] - target_position_state[0])**2
    dist += abs(complete_data[1] - target_position_state[1])**2
    dist = np.sqrt(dist)
    print('dist', dist)
    if dist >= 0.25:
        return -100. * dist
    else:
        return -10. * dist

# Reward difference between current and previous distance to target.
def reward_delta_dist_1(complete_data):
    global prev_dist

    target_pos = [complete_data[22], complete_data[23]]

    # Compute current distance from target position state.
    dist = np.sqrt(abs(complete_data[0] - target_pos[0])**2 + abs(complete_data[1] - target_pos[1])**2)

    # Compute difference between current and previous distance to target.
    delta_dist = dist - prev_dist

    # Give lowest reward if robot moves away from target
    if dist - prev_dist >= 0.:
        reward = -100

    # Give medium-high reward if robot gets closer to target
    else:
        reward = 10

    # Store current distance
    prev_dist = dist

    return reward

# Reward difference between current and previous distance to target (with target radius).
def reward_delta_dist_2(complete_data):
    global prev_dist, target_radius

    target_pos = [complete_data[22], complete_data[23]]

    # Compute current distance from target position state.
    dist = np.sqrt(abs(complete_data[0] - target_pos[0])**2 + abs(complete_data[1] - target_pos[1])**2)

    # Compute difference between current and previous distance to target.
    delta_dist = dist - prev_dist

    # If robot close to target position state: Higher reward
    if dist <= target_radius:
        if delta_dist > 0.:
            reward = -100
        else:
            reward = 100

    # If robot far away from target position: Lower reward
    else:
        if delta_dist > 0.:
            reward = -100
        else:
            reward = -10
        
    # Store current distance
    prev_dist = dist

    return reward

# Reward difference between current and previous distance to target (with target radius, and specific values for stabilised positions).
def reward_delta_dist_3(complete_data):
    global prev_dist

    target_pos = [complete_data[22], complete_data[23]]

    # Define delta distance threshold (i.e., upon which the robot would be considered as stable, or not).
    delta_dist_threshold = 0.05

    # Compute current distance from target position state.
    dist = np.sqrt(abs(complete_data[0] - target_pos[0])**2 + abs(complete_data[1] - target_pos[1])**2)

    # Compute difference between current and previous distance to target.
    delta_dist = dist - prev_dist

    # If robot close to target state:
    if complete_data[24] == 1.:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = -100

        # Give highest reward if robot stands still close to target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = 100

        # Give medium-high reward if robot gets closer to target
        elif delta_dist <= - delta_dist_threshold:
            reward = 10

    # If robot far away from target position:
    else:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = -100

        # Give lowest reward if robot stands still far away from target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = -100

        # Give medium-low reward if robot gets closer to target
        elif delta_dist <= - delta_dist_threshold:
            reward = -10
    
    # Store current distance
    prev_dist = dist

    return reward

# Reward silence.
def reward_sound_level(complete_data):
    accumulated_sound_level = complete_data[25]

    # Define sound level threshold.
    sound_level_threshold = 10.

    # Give lowest reward if sound level accumulated during one time step
    if accumulated_sound_level > sound_level_threshold:
        reward = - 100

    # Give highest reward if silence during one time step
    else:
        reward = 100

    return reward

def reward_inv_revolutions(complete_data):
    print("n.revolutions: {} ({})".format(complete_data[9], complete_data))
    return -abs(complete_data[9])
#    return 1.0/(1+abs(complete_data[9]))

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
    allmax = np.argwhere(prediction == np.amax(prediction)).flatten()
    return np.asscalar(np.random.choice(allmax, 1))
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
    return np.asscalar(np.random.choice(np.arange(len(prediction)), 1, p=prediction))
def robot_action(speed, steer):
    global last_nonzero_speed
    if speed != 0:
        last_nonzero_speed = speed
    client.send_message("/morphoses/action", [speed, steer])

def robot_next():
    client.send_message("/morphoses/next", [])

def robot_begin():
    client.send_message("/morphoses/begin", [])

def robot_end():
    robot_rgb(0, 0, 0)
    robot_action(0, 0)
    client.send_message("/morphoses/end", [])

def robot_rgb(r, g, b):
    client.send_message("/morphoses/rgb", [r, g, b])


if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("-p", "--policy", type=str, default="greedy", choices=["greedy", "boltzmann", "mixed"], help="Agent policy")
    parser.add_argument("-eps", "--epsilon", type=float, default=0.1, help="Epsilon value for the 'greedy' policy")
    parser.add_argument("-temp", "--temperature", type=float, default=1, help="Temperature value for the 'boltzmann' policy [0, +inf] (higher: more uniform, lower: more greedy)")

    parser.add_argument("-gam", "--gamma", type=float, default=0.95, help="Gamma value for the Q-learning")
    parser.add_argument("-lr", "--learning-rate", type=float, default=0.01, help="The learning rate")

    parser.add_argument("-cw", "--curiosity-weight", type=float, default=0.5, help="Weight of curiosity intrinsic reward (as %%)")

    parser.add_argument("-t", "--time-step", type=float, default=0, help="Period (in seconds) of each step (0 = as fast as possible)")
    parser.add_argument("--time-balance", type=float, default=0, help="Duration (in seconds) of each pause between each action (0 = no pause)")

    # Arguments for Neural networks.
    parser.add_argument("-nq", "--n-hidden-q", type=int, default=64, help="Number of hidden units per layer for the Q-function")
    parser.add_argument("-nf", "--n-hidden-forward", type=int, default=64, help="Number of hidden units per layer for the forward function")

    # Arguments for tile coding.
    parser.add_argument("-m", "--model", type=str, default="ann", choices=["ann", "ann-tiles", "tables"], help="Model type")
    parser.add_argument("--use-tile-coding", default=False, action='store_true', help="Use tile coding for Q function")
    parser.add_argument("--n-state-tiles", type=int, default=10, help="Number of tiles to use for each state dimension")
    parser.add_argument("--n-state-tilings", type=int, default=1, help="Number of tilings to use for each state dimension")

    parser.add_argument("--use-robot", default=False, action='store_true', help="Use real robot")

    parser.add_argument("--n-action-bins", type=int, default=3, help="Number of bins to use for each action dimension")

    parser.add_argument("--plot", default=False, action='store_true', help="Plot figures")
    parser.add_argument("--use-spec-actions", default=False, action='store_true', help="Use specific speed/steering actions (i.e., not necessarily computed using bin division)")
    parser.add_argument("--use-tag-target", default=False, action='store_true', help="Use RTLS tag as target state")

    parser.add_argument("--use-position", default=False, action='store_true', help="Add position to the input data")
    parser.add_argument("--use-delta-position", default=False, action='store_true', help="Add delta position to the input data")
    parser.add_argument("--use-quaternion", default=False, action='store_true', help="Add quaternion to the input data")
    parser.add_argument("--use-delta-quaternion", default=False, action='store_true', help="Add delta quaternion to the input data")
    parser.add_argument("--use-euler", default=False, action='store_true', help="Add Euler angles to the input data")
    parser.add_argument("--use-euler-firsts", default=False, action='store_true', help="Add first two Euler angles to the input data")
    parser.add_argument("--use-euler-extremes", default=False, action='store_true', help="Add first and last Euler angles to the input data")
    parser.add_argument("--use-delta-euler", default=False, action='store_true', help="Add delta Euler angles to the input data")
    parser.add_argument("--use-n-revolutions", default=False, action='store_true', help="Add number of revolutions the input data")
    parser.add_argument("--use-orientation", default=False, action='store_true', help="Add robot orientation to the input data")
    parser.add_argument("--use-direction-to-target", default=False, action='store_true', help="Add direction to target to the input data")
    parser.add_argument("--use-polar-coordinates-to-target", default=False, action='store_true', help="Add polar coordinates to target (distance and direction) to the input data")
    parser.add_argument("--use-constant-state", default=False, action='store_true', help="Add constant state to the input data (e.g., no state space, only actions)")

    parser.add_argument("--reward-position-center", default=False, action='store_true', help="Reward being close to center (0, 0)")
    parser.add_argument("--reward-inv-position-border", default=False, action='store_true', help="Punish heavily being too close to the edges")
    parser.add_argument("--reward-delta-euler", default=False, action='store_true', help="Reward Euler motion")
    parser.add_argument("--reward-inv-delta-euler", default=False, action='store_true', help="Reward no Euler motion")
    parser.add_argument("--reward-delta-roll", default=False, action='store_true', help="Reward roll motion")
    parser.add_argument("--reward-inv-delta-roll", default=False, action='store_true', help="Reward no roll motion")
    parser.add_argument("--reward-delta-pitch", default=False, action='store_true', help="Reward pitch motion")
    parser.add_argument("--reward-inv-delta-pitch", default=False, action='store_true', help="Reward no pitch motion")
    parser.add_argument("--reward-delta-yaw", default=False, action='store_true', help="Reward yaw motion")
    parser.add_argument("--reward-inv-delta-yaw", default=False, action='store_true', help="Reward no yaw motion")

    parser.add_argument("--reward-euler-state-1", default=False, action='store_true', help="Reward static Euler state using 1st experiment reward")
    parser.add_argument("--reward-euler-state-2", default=False, action='store_true', help="Reward static Euler state using 2nd experiment reward")
    parser.add_argument("--reward-euler-state-3", default=False, action='store_true', help="Reward static Euler state using 3rd experiment reward")
    parser.add_argument("--reward-euler-state-robot-1", default=False, action='store_true', help="Reward static robot Euler state using 1st experiment reward")
    parser.add_argument("--reward-position-state", default=False, action='store_true', help="Reward static position state")
    parser.add_argument("--reward-inv-revolutions", default=False, action='store_true', help="Reward low number of revolutions")
    parser.add_argument("--reward-delta-dist-1", default=False, action='store_true', help="Reward difference between current and previous distance to target")
    parser.add_argument("--reward-delta-dist-2", default=False, action='store_true', help="Reward difference between current and previous distance to target (with target radius)")
    parser.add_argument("--reward-delta-dist-3", default=False, action='store_true', help="Reward difference between current and previous distance to target (with target radius, and specific values for stabilised positions)")
    parser.add_argument("--reward-sound-level", default=False, action='store_true', help="Reward silence")
    
    parser.add_argument("--target-radius", type=float, default=0.2, help="Radius (in m) from the target in which the robot would receive high reward")

    parser.add_argument("--x-min", type=float, default=-math.inf, help="Left boundary (x) of the virtual fence (math.inf = no fence)")
    parser.add_argument("--x-max", type=float, default=math.inf, help="Right boundary (x) of the virtual fence (math.inf = no fence)")
    parser.add_argument("--y-min", type=float, default=-math.inf, help="Bottom boundary (y) of the virtual fence (math.inf = no fence)")
    parser.add_argument("--y-max", type=float, default=math.inf, help="Top boundary (y) of the virtual fence (math.inf = no fence)")

#    parser.add_argument("-D", "--model-directory", type=str, defaultsi help="The directory where to save models")
#    parser.add_argument("-P", "--prefix", type=str, default="ann-regression-", help="Prefix to use for saving files")

    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to send data to.")
    parser.add_argument("-s", "--send-port", default="8765",
                        type=int, help="Specify the port number to send data to.")
    parser.add_argument("-r", "--receive-port", default="8767",
                        type=int, help="Specify the port number to listen on.")

    # Parse arguments.
    args = parser.parse_args()

    notify_recv = False
    perf_measurements = []
    rows = 0

    plot = args.plot
    use_spec_actions = args.use_spec_actions
    use_tag_target = args.use_tag_target

    n_action_bins = args.n_action_bins
    n_actions = n_action_bins*n_action_bins

    time_step = args.time_step
    time_balance = args.time_balance

    target_radius = args.target_radius

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
    if args.use_n_revolutions:
        state_columns += [9]
    if args.use_euler_firsts:
        state_columns += [6, 7]
    if args.use_euler_extremes:
        state_columns += [6, 8]
    if args.use_delta_position:
        state_columns += [10, 11]
    if args.use_delta_quaternion:
        state_columns += [12, 13, 14, 15]
    if args.use_delta_euler:
        state_columns += [16, 17, 18]
    if args.use_orientation:
        state_columns += [20]
    if args.use_direction_to_target:
        state_columns += [21]
    if args.use_polar_coordinates_to_target:
        state_columns += [24, 21]
    if args.use_constant_state:
        state_columns += [26]
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
    if args.reward_delta_roll:
        extrinsic_reward_functions += [reward_delta_roll]
    if args.reward_inv_delta_roll:
        extrinsic_reward_functions += [reward_inv_delta_roll]
    if args.reward_delta_pitch:
        extrinsic_reward_functions += [reward_delta_pitch]
    if args.reward_inv_delta_pitch:
        extrinsic_reward_functions += [reward_inv_delta_pitch]
    if args.reward_delta_yaw:
        extrinsic_reward_functions += [reward_delta_yaw]
    if args.reward_inv_delta_yaw:
        extrinsic_reward_functions += [reward_inv_delta_yaw]
    if args.reward_euler_state_1:
        extrinsic_reward_functions += [reward_euler_state_1]
    if args.reward_euler_state_2:
        extrinsic_reward_functions += [reward_euler_state_2]
    if args.reward_euler_state_3:
        extrinsic_reward_functions += [reward_euler_state_3]
    if args.reward_euler_state_robot_1:
        extrinsic_reward_functions += [reward_euler_state_robot_1]
    if args.reward_position_state:
        extrinsic_reward_functions += [reward_position_state]
    if args.reward_inv_revolutions:
        extrinsic_reward_functions += [reward_inv_revolutions]
    if args.reward_delta_dist_1:
        extrinsic_reward_functions += [reward_delta_dist_1]
    if args.reward_delta_dist_2:
        extrinsic_reward_functions += [reward_delta_dist_2]
    if args.reward_delta_dist_3:
        extrinsic_reward_functions += [reward_delta_dist_3]
    if args.reward_sound_level:
        extrinsic_reward_functions += [reward_sound_level]

    # Configure virtual fence.
    x_min = args.x_min
    x_max = args.x_max
    y_min = args.y_min
    y_max = args.y_max

    use_ann = not args.model == "tables"
    use_tile_coding = not args.model == "ann"

    # Tile coding.
    if use_tile_coding:
        tile_coding = rep.TileCoding(input_indices = [np.arange(n_inputs)],
						             ntiles = [args.n_state_tiles],
						             ntilings = [args.n_state_tilings],
						             hashing = None,
                                     bias_term = False,
						             state_range = [np.full(n_inputs, 0), np.full(n_inputs, 1)],
						             rnd_stream = np.random.RandomState())
        n_inputs_q = tile_coding.size
    else:
        tile_coding = None
        n_inputs_q = n_inputs

    # Initialize stuff.
    n_inputs_forward = n_inputs + n_actions

    n_hidden_q = args.n_hidden_q
    n_hidden_forward = args.n_hidden_forward

    learning_rate = args.learning_rate

    # Compile model Q(state_t, action_t)
    if use_ann:
        model_q = Sequential()
        model_q.add(InputLayer(batch_input_shape=(1, n_inputs_q)))
        if (n_hidden_q > 0):
            model_q.add(Dense(n_hidden_q, activation='relu'))
        model_q.add(Dense(n_actions, activation='linear'))
        # model_q.add(Dense(n_actions, activation='softmax'))
        model_q.compile(loss='categorical_crossentropy', optimizer=optimizers.SGD(lr=learning_rate), metrics=['accuracy'])
        print(model_q.summary())
    else:
        model_q = np.zeros((n_inputs_q, n_actions))

    # Predicts state_{t+1} = f(state_t, action_t)
    model_forward = Sequential()
    model_forward.add(InputLayer(batch_input_shape=(1, n_inputs_forward)))
    if (n_hidden_forward > 0):
        model_forward.add(Dense(n_hidden_forward, activation='relu'))
    model_forward.add(Dense(n_inputs, activation='linear'))
    model_forward.compile(loss='mse', optimizer='adam', metrics=['mae'])
    print(model_forward.summary())

    # Initialize stuff.
    prev_data = None
    prev_time = -1
    prev_state = None
    prev_action = 0 # dummy
    avg_r = None
    max_r = -1000
    min_r = 1000
    count_action = np.zeros(n_actions)
    #avg_r = np.array([ 0., 0., 0. ])

    prev_corr_action = None
    prev_dist = 0.
    constant_state = 0

    # Initialize target state
    if use_tag_target:
        target_position_state = [0., 0.] # dummy
    else:
        target_position_state = [0.5, 0.5]

    # Create rescalers.
    scalerX = MinMaxScaler()
    scalerX.fit([[x_min, y_min, -1, -1, -1, -1, -180, -90, -180, -5.],
                 [x_max, y_max, +1, +1, +1, +1, +180, +90, +180,  5.]])

    scalerY = MinMaxScaler()
    scalerY.fit([[-1, 1],
                 [-1, 1]])
    #scalerY.fit([[-15, -45],
    #             [+15, +45]])

    plt.ion()
    plt.show()

    iter = 0

    # Keep track of last value of nonzero speed
    last_nonzero_speed = 1 # dummy value

    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed_ticks, speed, steer, tag_x, tag_y, accumulated_sound_level):
        global notify_recv, use_ann
        global prev_data, prev_time, prev_state, prev_action, prev_corr_action, plot
        global avg_r, iter, max_r, min_r, count_action
        global last_nonzero_speed

#        start_time = time.perf_counter()

        if not(notify_recv):
            print("Received initial packets.")
            notify_recv = True

        # Process input data.
        pos = np.array([x, y]) # 0, 1
        quat = np.array([qx, qy, qz, qw]) # 2, 3, 4, 5
        euler = np.array(mpp.quaternion_to_euler(qx, qy, qz, qw)) # 6, 7, 8
        ticks = np.array([speed_ticks]) # 9
        data = np.concatenate((pos, quat, euler, ticks))
        data = mpp.standardize(data, scalerX)[0] # normalize

        # Update target position
        tag_pos = np.array([tag_x, tag_y])
        temp_data = np.concatenate((tag_pos, quat, euler, ticks)) # rough: fill array with false data just to apply scalerX.
        temp_data = mpp.standardize(temp_data, scalerX)[0]
        target_position_state = [temp_data[0], temp_data[1]]

        print('------------------------------')

        # If the robot is within virtual fence: Perform standard RL loop.
        if x_min < x < x_max and y_min < y < y_max:
            print('Inside virtual fence')
            prev_corr_action = None

            # If this is the first time we receive something: save as initial values and skip
            if prev_data is None:
                # Reset.
                prev_data = data
                prev_time = t
                delta_data = delta(data, data) # ie. 0
                orientation = 0.
                direction_to_target = 0
                distance_to_target = 0
                complete_data = np.concatenate((data, delta_data))
                complete_data = np.append(complete_data, orientation)
                complete_data = np.append(complete_data, direction_to_target)
                complete_data = np.append(complete_data, target_position_state[0])
                complete_data = np.append(complete_data, target_position_state[1])
                complete_data = np.append(complete_data, distance_to_target)
                complete_data = np.append(complete_data, accumulated_sound_level)
                complete_data = np.append(complete_data, constant_state)
                state = get_state(complete_data, state_columns)
                prev_state = state
                prev_action = 0 # dummy
                r = 0

            # Else: one step of Q-learning loop.
            else:
                # Compute state.
                delta_data = 10 * delta(data, prev_data) / (t - prev_time)
    #            delta_data = 100 * delta(data, prev_data) / (t - prev_time)
                complete_data = np.concatenate((data, delta_data))

                orientation = compute_orientation(delta_data, last_nonzero_speed)
                direction_to_target = compute_direction_to_target(data, delta_data, target_position_state, last_nonzero_speed, plot)
                distance_to_target = compute_distance_to_target(data, target_position_state, target_radius)

                complete_data = np.append(complete_data, orientation)
                complete_data = np.append(complete_data, direction_to_target)
                complete_data = np.append(complete_data, target_position_state[0])
                complete_data = np.append(complete_data, target_position_state[1])
                complete_data = np.append(complete_data, distance_to_target)
                complete_data = np.append(complete_data, accumulated_sound_level)
                complete_data = np.append(complete_data, constant_state)

                state = get_state(complete_data, state_columns)
                # print("complete data : {} {}".format(complete_data, state_columns))

                # Adjust state model.
                state_model_input = np.concatenate((prev_state[0], to_categorical(prev_action, n_actions)))
                state_model_input = np.reshape(state_model_input, (1, n_inputs_forward))
    #            print(state_model_input)
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
                if r > max_r:
                    max_r = r
                if r < min_r:
                    min_r = r
                scaled_r = (r - min_r) / (max_r - min_r + 0.000001)
                brightness = round(scaled_r * 255)
                robot_rgb(255 - brightness, brightness, 0)

    #            print(r)

                r_array = np.array([ r_int, r_ext, r ])

                if avg_r is None:
                    avg_r = r_array
                else:
                    avg_r -= (1-gamma) * (avg_r - r_array)

                # Save previous data information.
                prev_data = data
                prev_time = t

                print("{} => {}".format(state, r))
                n_iter_log = 10
                if iter % n_iter_log == 0:
                    print("t={} average reward = (int: {} ext: {} total: {})".format(iter, avg_r[0], avg_r[1], avg_r[2]))
                    print("state = ", state)
                    print("counts = ", count_action / sum(count_action))
                    avg_r = r_array # reset


            # print("State: {} => Coding: {}".format(state, state_to_tile_coding(state, tile_coding)))
            # Get prediction table.

            if use_ann:
                prediction = model_q.predict(state_to_tile_coding(state, tile_coding), verbose=0).squeeze()
            else:
                prediction = q_table_predict(model_q, state, tile_coding)

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

            count_action[action] += 1

            use_sarsa = True
            # Perform one step.
            if use_sarsa:
                target = r + gamma * prediction[action]
            else:
                target = r + gamma * np.max(prediction)

            # Perform one step.
            # Source: https://keon.io/deep-q-learning/
            # learned value = r + gamma * max_a Q(s_{t+1}, a)
            if use_ann:
                target_vec = model_q.predict(state_to_tile_coding(prev_state, tile_coding))[0] # Q(s_t, a_t)
                target_vec[prev_action] = target
                model_q.fit(state_to_tile_coding(prev_state, tile_coding), target_vec.reshape(-1, n_actions), epochs=1, verbose=0)
            else:
                target_vec = q_table_predict(model_q, prev_state, tile_coding)
                q_table_update(model_q, state, tile_coding, target, prev_action, learning_rate)

            # Save action for next iteration.
            prev_action = action

            if use_spec_actions == True:
                target = [mpp.class_to_speed_steering_spec(action, n_action_bins, True)]
            else:
                target = [mpp.class_to_speed_steering(action, n_action_bins, True)]
            #target = [[0.5, 1]] # 0.5 correponds to zero speed/steer.
           # print('target ', target)
    #        print("Choice made {} {} {}".format(action, model.predict(state), target))

            # Send OSC message of action.
            action = target
            #action = scalerY.inverse_transform(target).astype('float')
            print("Target {} Action {}".format(target, action))

            # Send OSC message.
            robot_action(action[0][0], action[0][1])
            #print('action', action[0])

            # Update state.
            prev_state = state

            iter += 1

            # Wait
            if time_step > 0:
                time.sleep(time_step)

            # Structure learning by having the robot take a pause in a stabilised state (zero speed, zero steering).
            if time_balance > 0:
                robot_action(0, 0)
                time.sleep(time_balance)

            # Ask for next data point.
            robot_next()

        # If the robot is outside virtual fence: Correct robot position.
        else:
            print('Outside virtual fence', 0)

            # Signal that I am out.
            robot_rgb(0, 0, 255)

            # Pause and stabilise the robot
            robot_action(0, 0)
            time.sleep(1)

            # If no correction was made on robot position, use previous action as previous correction action.
            if prev_corr_action is None:
                if use_spec_actions == True:
                    target = [mpp.class_to_speed_steering_spec(prev_action, n_action_bins, True)]
                else:
                    target = [mpp.class_to_speed_steering(prev_action, n_action_bins, True)]
                action = target
                #action = scalerY.inverse_transform(target).astype('float')
                prev_corr_action = action

            # Get signs of previous speed and steer actions.
            sgn_speed = np.sign(prev_corr_action[0][0])
            sgn_steer = np.sign(prev_corr_action[0][1])

            # If previous speed action was zero, take random speed action during short time. (Could be improved...)
            if sgn_speed == 0:
                corr_speed = random.choice([-1., 1.])
                time_corr_action = 2

            # Else, correct speed as opposite previous speed action during long time.
            else:
                corr_speed = -1. * prev_corr_action[0][0]
                time_corr_action = 6

            # If previous steer action was zero, choose random steer action. (Could be improved...)
            if sgn_steer == 0:
                corr_steer = random.choice([-1., 1.]) * 0.5

            # Else, correct steer as opposite previous steer action.
            else:
                corr_steer = -1. * prev_corr_action[0][1]

            corr_action = [[corr_speed, corr_steer]]
            print('corr_action', corr_action)

            # Send corrective action to robot.
            robot_action(corr_action[0][0], corr_action[0][1])

            # Store previous corrective action.
            prev_corr_action = corr_action

            # Wait
            time.sleep(time_corr_action)

            # Pause and stabilise the robot
            robot_action(0, 0)
            time.sleep(1)

            # Ask for next data point.
            robot_next()

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server & client.

    server = osc_server.BlockingOSCUDPServer(("0.0.0.0", args.receive_port), dispatcher)
    client = udp_client.SimpleUDPClient(args.ip, args.send_port)

    def interrupt(signup, frame):
        global client, server
        print("Exiting program... {np.mean(perf_measurements)}")
        robot_end()
        server.server_close()
        sys.exit()

    signal.signal(signal.SIGINT, interrupt)

    print("Serving on {}. Program ready.".format(server.server_address))
    robot_begin()
    if args.use_robot:
        time.sleep(10) # Give time to the robot to do its starting sequence
    print("Go!")

    server.serve_forever()
