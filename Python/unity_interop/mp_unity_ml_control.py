import argparse
import sys
import time

import pandas
import numpy as np
from keras.models import load_model
from sklearn.preprocessing import MinMaxScaler

from pythonosc import dispatcher
from pythonosc import osc_server

from pythonosc import osc_message_builder
from pythonosc import udp_client



def quaternion_to_euler(x, y, z, w):
    import math
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + y * y)
    X = math.degrees(math.atan2(t0, t1))

    t2 = +2.0 * (w * y - z * x)
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    Y = math.degrees(math.asin(t2))

    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (y * y + z * z)
    Z = math.degrees(math.atan2(t3, t4))

    return X, Y, Z    

if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()
    parser.add_argument("model_file", type=str, help="File containing the trained model")
    parser.add_argument("data", type=str, help="The CSV file containing the dataset on which the model was trained")
    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to send data to.")
    parser.add_argument("-s", "--send-port", default="8765",
                        type=int, help="Specify the port number to listen on.")
    parser.add_argument("-r", "--receive-port", default="8767",
                        type=int, help="Specify the port number to listen on.")

    # Parse arguments.
    args = parser.parse_args()

    # Load model.
    model = load_model(args.model_file)
    model._make_predict_function()

    notify_recv = False
    rows = 0

    # Load database.
    dataframe = pandas.read_csv(args.data)
    dataset = dataframe.values

    # Create input matrix X.
    pos_X = np.array(dataset[:,2:4],dtype='float64')
    quat_X = np.array(dataset[:,4:8],dtype='float64')
    euler_X = np.matrix([ quaternion_to_euler(q[0], q[1], q[2], q[3]) for q in quat_X ])

    # Join blocks.
    X = np.concatenate((pos_X, quat_X, euler_X), axis=1)

    # Normalize X.
    scalerX = MinMaxScaler()
    scalerX.fit(X)
    X = scalerX.transform(X)

    # Create target matrix Y.
    speed_y = np.array(dataset[:,8],dtype='float64')
    steering_y = np.array(dataset[:,9],dtype='float64')

    # Join blocks.
    Y = np.column_stack((speed_y, steering_y))

    # Normalize Y.
    scalerY = MinMaxScaler()
    scalerY.fit(Y)
    Y = scalerY.transform(Y)

    def standardize_data(data):
        global scalerX
        print("Standardizeing")
        print(data)
        data = scalerX.transform([data])
        return data


    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed, steer):
        global notify_recv
        global model
        if not(notify_recv):
            print("Recieved initial packets from unity!")
            notify_recv = True

        # Process input data.
        pos = np.array([x, y])
        quat = np.array([qx, qy, qz, qw])
        euler = np.array(quaternion_to_euler(qx, qy, qz, qw))
        data = np.concatenate((pos, quat, euler))
        data = standardize_data(data)

        # Generate prediction.
        target = model.predict(data)
        action = scalerY.inverse_transform(target).astype('float')
        print("Chosen action: ")
        print(action)

        # Send OSC message.
        client.send_message("/morphoses/action", action[0])
        time.sleep(0.0005) # necessary???

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server & client.
    server = osc_server.ThreadingOSCUDPServer((args.ip, args.receive_port), dispatcher)
    client = udp_client.SimpleUDPClient(args.ip, args.send_port)

    print(f"Serving on {server.server_address}")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Exiting program...")
        server.server_close()
        sys.exit()
