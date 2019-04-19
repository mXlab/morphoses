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

if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to send data to.")
    parser.add_argument("-s", "--send-port", default="8765",
                        type=int, help="Specify the port number to listen on.")
    parser.add_argument("-r", "--receive-port", default="8767",
                        type=int, help="Specify the port number to listen on.")
    parser.add_argument("--n-bins", type=int, default=3,
                        help="Number of bins to use for classification (only valid if used with --classification)")

    # Parse arguments.
    args = parser.parse_args()

    notify_recv = False
    perf_measurements = []
    rows = 0

    n_bins = args.n_bins
    n_actions = n_bins*n_bins
    n_inputs = 6
    n_hidden = 10

    model = Sequential()
    model.add(InputLayer(batch_input_shape=(1, n_inputs)))
    model.add(Dense(n_hidden, activation='relu'))
    model.add(Dense(n_actions, activation='sigmoid'))
    model.compile(loss='mse', optimizer='adam', metrics=['mae'])

    prev_data = None
    prev_time = -1
    prev_state = None
    prev_action = -1
    eps = 0.1
    y = 0.1
#    y = 0.95

    avg_r = None

    scalerX = MinMaxScaler()
    scalerX.fit([[-25, -25, -1, -1, -1, -1, -180, -90, -180],
                 [+25, +25, +1, +1, +1, +1, +180, +90, +180]])

    scalerY = MinMaxScaler()
    scalerY.fit([[-15, -45],
                 [+15, +45]])

    iter = 0
    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed, steer):
        global notify_recv
        global model
        global perf_measurements
        global prev_data, prev_time, prev_state, prev_action
        global avg_r, iter

        start_time = time.perf_counter()

        if not(notify_recv):
            print("Received initial packets from unity!")
            notify_recv = True

        # Process input data.
        pos = np.array([x, y])
        quat = np.array([qx, qy, qz, qw])
        euler = np.array(mpp.quaternion_to_euler(qx, qy, qz, qw))
        data = np.concatenate((pos, quat, euler))

        data = mpp.standardize(data, scalerX)[0]

        # If this is the first time we receive something: save as initial values and skip
        if prev_data is None:
            # Reset.
            prev_data = data
            prev_time = t
            state = np.concatenate((data[6:9], [ 0, 0, 0 ]))
            state = np.reshape(state, (1, 6))
            prev_state = state
            prev_action = 0
        # Else: one step of Q-learning loop.
        else:
            # Compute deltas.
            delta_data = delta(data, prev_data) / (t - prev_time)

            # Create state vector.
            state = np.concatenate((data[6:9], delta_data[6:9])) # euler + d_euler
            state = np.reshape(state, (1, 6))

            # Reward (equal to inverse sum of motion).
            r = 1 - abs(sum(delta_data[6:9]))

            if avg_r is None:
                avg_r = r
            else:
                avg_r -= 0.01 * (avg_r - r)

            # Save previous data information.
            prev_data = data
            prev_time = t

            # Perform one step.
            target = r + y * np.max(model.predict(state))
            target_vec = model.predict(prev_state)[0]
            target_vec[prev_action] = target
            model.fit(prev_state, target_vec.reshape(-1, n_actions), epochs=1, verbose=0)

        # Choose action.
        if np.random.random() < eps:
            action = np.random.randint(n_actions)
        else:
            action = np.argmax(model.predict(state))

        # Save action for next iteration.
        prev_action = action

        target = [mpp.class_to_speed_steering(action, n_bins, True)]
#        print("Choice made {} {} {}".format(action, model.predict(state), target))

        # Send OSC message of action.
        action = scalerY.inverse_transform(target).astype('float')
#        print("Target {} Action {}".format(target, action))
        if iter % 1000 == 0:
            print("t={} average reward = {})".format(iter, avg_r))

        # Send OSC message.
        client.send_message("/morphoses/action", action[0])

        # Update state.
        prev_state = state

        iter += 1

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
