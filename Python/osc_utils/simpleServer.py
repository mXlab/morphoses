"""Small example OSC server

This program listens to several addresses, and prints some information about
received packets.
"""
import argparse
import math

from pythonosc import dispatcher
from pythonosc import osc_server

recievedMessage = ""

def print_volume_handler(unused_addr, args, volume):
    print(args[0])
    print(volume)
    print("[{0}] ~ {1}\n".format(args[0], volume))

def splitMessage():
    return 0

def recieveOSCData(unused_addr, args, q1, q2, q3, q4):
    #recievedMessage = msg
    print("{} {} {} {}".format(q1, q2, q3, q4))

def test(msg):
    print("yas I'm working!")
    print(msg)

'''
def print_compute_handler(unused_addr, args, volume):
  try:
    print("[{0}] ~ {1}".format(args[0], args[1](volume)))
  except ValueError: passmsg
'''


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip",
      default="127.0.0.1", help="The ip to listen on")
  parser.add_argument("--port",
      type=int, default=5005, help="The port to listen on")
  args = parser.parse_args()

  dispatcher = dispatcher.Dispatcher()
  #dispatcher.map("/filter", print)
  #dispatcher.map("/volume", print_mytest, "lookatme")
  dispatcher.map("/quat", recieveOSCData, "idkwhatthisisfor")
  #dispatcher.map("/logvolume", print_compute_handler, "Log volume", math.log)

  server = osc_server.ThreadingOSCUDPServer(
      (args.ip, args.port), dispatcher)

  print("Serving on {}".format(server.server_address))
  print(recievedMessage)

  '''
  for ap, arg in dispatcher._map.items():
      print(ap)
      print(arg)
  '''

  server.serve_forever()
