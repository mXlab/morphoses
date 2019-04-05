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

# This is to allow the inclusion of common python code in the ../mp folder
import sys
sys.path.append('../')

import mp.preprocessing as mpp

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
    perf_measurements = []
    rows = 0

    # Load database.
    dataframe = pandas.read_csv(args.data)
    dataset = dataframe.values

    # Pre-process.
    X, Y, scalerX, scalerY = mpp.preprocess_data(dataset, prune_experiments=True)

    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed, steer):
        global notify_recv
        global model
        global perf_measurements
        start_time = time.perf_counter()

        if not(notify_recv):
            print("Recieved initial packets from unity!")
            notify_recv = True

        # Process input data.
        pos = np.array([x, y])
        quat = np.array([qx, qy, qz, qw])
        euler = np.array(mpp.quaternion_to_euler(qx, qy, qz, qw))
        data = np.concatenate((pos, quat, euler))
        data = mpp.standardize(data, scalerX)

        # Generate prediction.
        target = model.predict(data)
        action = scalerY.inverse_transform(target).astype('float')
        #print("Chosen action: ")
        #print(action)

        # Send OSC message.
        client.send_message("/morphoses/action", action[0])
        perf_measurements.append(time.perf_counter() - start_time)

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server & client.
    server = osc_server.ThreadingOSCUDPServer((args.ip, args.receive_port), dispatcher)
    client = udp_client.SimpleUDPClient(args.ip, args.send_port)

    print(f"Serving on {server.server_address}. Program ready.")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Exiting program...")
        server.server_close()
        sys.exit()
