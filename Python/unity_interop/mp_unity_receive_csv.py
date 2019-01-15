import argparse
import math
import csv

from pythonosc import dispatcher
from pythonosc import osc_server


if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()
    parser.add_argument("output_file", type=str, help="CSV output file")
    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to listen on.")
    parser.add_argument("-p", "--port", default="8767",
                        type=int, help="Specify the port number to listen on.")

    # Parse arguments.
    args = parser.parse_args()

    # Create CSV file.
    csv_file = open(args.output_file, "w")
    csv_writer = csv.writer(csv_file)

    def handle_data(unused_addr, id, t, x, y, qx, qy, qz, qw):
        csv_writer.writerow([id, t, x, y, qx, qy, qz, qw])

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server.
    server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)

    print(f"Serving on {server.server_address}")
    server.serve_forever()
