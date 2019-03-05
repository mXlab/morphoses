import argparse
import math
import csv
import sys

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
    csv_writer = csv.DictWriter(
        csv_file, ["id", "time", "x", "y", "qx", "qy", "qz", "qw", "speed", "steer"])
    csv_writer.writeheader()

    notify_recv = False
    rows = 0

    def handle_data(unused_addr, exp_id, t, x, y, qx, qy, qz, qw, speed, steer):
        global notify_recv
        global rows
        if not(notify_recv):
            print("Recieved initial packets from unity!")
            notify_recv = True
        csv_writer.writerow({
            "id": exp_id,
            "time": t,
            "x": x,
            "y": y,
            "qx": qx,
            "qy": qy,
            "qz": qz,
            "qw": qw,
            "speed": speed,
            "steer": steer
        })
        rows += 1

    # Create OSC dispatcher.
    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/morphoses/data", handle_data)

    # Launch OSC server.
    server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)

    print(f"Serving on {server.server_address}")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Exiting program...")
        print(f"saved {rows} rows of data")
        csv_file.close()
        server.server_close()
        sys.exit()
