import argparse
import math

from pythonosc import dispatcher
from pythonosc import osc_server

def print_volume_handler(unused_addr, args, volume):
  print("[{0}] ~ {1}".format(args[0], volume))

def print_compute_handler(unused_addr, args, volume):
  try:
    print("[{0}] ~ {1}".format(args[0], args[1](volume)))
  except ValueError: pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("-ip", "--ip", default="127.0.0.1",
        help="Specify the ip address to listen on. Default: 127.0.0.1")

    parser.add_argument("-p", "--port", default="8767", type=int,
        help="Specify the port number to listen on. Default: 8767")

    args = parser.parse_args()

    dispatcher = dispatcher.Dispatcher()
    dispatcher.map("/quat", print)
    dispatcher.map("/volume", print_volume_handler, "Volume")
    dispatcher.map("/logvolume", print_compute_handler, "Log volume", math.log)

    server = osc_server.ThreadingOSCUDPServer(
        (args.ip, args.port), dispatcher)

    print("Serving on {}".format(server.server_address))
    server.serve_forever()
