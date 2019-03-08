import argparse
import math
import csv
import time

from pythonosc import osc_message_builder
from pythonosc import udp_client

if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()

    parser.add_argument("input_file", type=str, help="CSV input file")
    parser.add_argument("-i", "--ip", default="127.0.0.1",
                        help="Specify the ip address to send data to.")
    parser.add_argument("-p", "--port", default="8765",
                        type=int, help="Specify the port number to send data to.")

    # Parse arguments.
    args = parser.parse_args()

    client = udp_client.SimpleUDPClient(args.ip, args.port)

    # Open CSV input file.
    csv_input_file = open(args.input_file, "r")
    csv_reader = csv.reader(csv_input_file, quoting=csv.QUOTE_NONNUMERIC)

    for row in csv_reader:
        # builder = osc_message_builder.OscMessageBuilder(address="/mophoses/data")

        client.send_message("/morphoses/transform", row)
        time.sleep(0.0005)
