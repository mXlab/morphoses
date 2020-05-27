import argparse
import time
import signal
import sys

from osc4py3.as_eventloop import *
from osc4py3 import oscbuildparse
from osc4py3 import oscmethod as osm

next_data_requested = False
start_time = time.time()
current_speed = 0
current_steer = 0
current_quaternion = [0, 0, 0, 0]
current_n_revolutions = 0

N_MOTOR1_TICKS_PER_REVOLUTION = 1365

class OscHelper:
    def __init__(self, name, ip, send_port, receive_port):
        print("Creating OSC link at IP {} send = {} recv = {}".format(ip, send_port, receive_port))
        self.name = name
        osc_udp_client(ip, int(send_port), self.client_name())
        osc_udp_server("0.0.0.0", int(receive_port), self.server_name())

    def client_name(self):
        return self.name + "_client"

    def server_name(self):
        return self.name + "_server"

    def map(self, path, function):
        osc_method(path, function, argscheme=osm.OSCARG_ADDRESS + osm.OSCARG_DATAUNPACK)

    def send_message(self, path, args):
        if not isinstance(args, list):
            args = [ args ]
        msg = oscbuildparse.OSCMessage(path, None, args)
        osc_send(msg, self.client_name())
        osc_process()

# Re-route action to robot.
def receive_action(unused_addr, speed, steer):
    global main_osc, current_speed, current_steer
    current_speed = speed
    current_steer = steer
    speed = 0.5 * (speed / 15 * 255)
    steer = steer * 0.75
    main_osc.send_message("/motor/1", round(speed))
    main_osc.send_message("/motor/2", round(steer))

# Main program asks for next data point.
def receive_next(unused_addr):
    global next_data_requested
    print("Requested next data")
    next_data_requested = True

def receive_begin(unused_addr):
    global next_data_requested
    main_osc.send_message("/power", 1)
    next_data_requested = True

def receive_end(unused_addr):
    global next_data_requested
    print("Requested next data")
    main_osc.send_message("/power", 0)
    # Switch motors off
    main_osc.send_message("/motor/1", 0)
    main_osc.send_message("/motor/2", 0)
    next_data_requested = False

def receive_rgb(unused_addr, r, g, b):
    global next_data_requested
    print("RGB: {} {} {}".format(r, g, b))
    main_osc.send_message("/red", int(r))
    main_osc.send_message("/green", int(g))
    main_osc.send_message("/blue", int(b))

# Preserve state value for next call.
def receive_quaternion(unused_addr, q0, q1, q2, q3):
    global current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested
    current_quaternion = [q0, q1, q2, q3]
    print("Received quaternion: {}".format([q0, q1, q2, q3]))
    if next_data_requested:
        bridge_osc.send_message("/morphoses/data", [ 0, time.time() - start_time, 0, 0, q0, q1, q2, q3, current_n_revolutions, current_speed, current_steer ])
        next_data_requested = False

# Preserve state value for next call.
def receive_speed_ticks(unused_addr, ticks):
    global current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested
    current_n_revolutions = ticks / N_MOTOR1_TICKS_PER_REVOLUTION
    q0 = current_quaternion[0]
    q1 = current_quaternion[1]
    q2 = current_quaternion[2]
    q3 = current_quaternion[3]
    if next_data_requested:
        bridge_osc.send_message("/morphoses/data", [ 0, time.time() - start_time, 0, 0, q0, q1, q2, q3, current_n_revolutions, current_speed, current_steer ])
        next_data_requested = False

# Create parser
parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument("--bridge-receive-port", default="7765",
                        help="Specify the port number where data is received from the main application.")
parser.add_argument("--bridge-send-port", default="7767",
                        help="Specify the port number to send to the main application.")

parser.add_argument("--main-board-ip", default="192.168.0.100",
                        help="Specify the ip address of the main board.")
parser.add_argument("--main-board-send-port", default="8765",
                        help="Specify the port number to send to main board.")
parser.add_argument("--main-board-receive-port", default="8766",
                        help="Specify the port number where data is received from the main board.")

parser.add_argument("--imu-board-ip", default="192.168.0.101",
                        help="Specify the ip address of the IMU board.")
parser.add_argument("--imu-board-send-port", default="8765",
                        help="Specify the port number to send to IMU board.")
parser.add_argument("--imu-board-receive-port", default="8767",
                        help="Specify the port number where data is received from the IMU board.")

args = parser.parse_args()

osc_startup()

main_osc = OscHelper("main", args.main_board_ip, args.main_board_send_port, args.main_board_receive_port)
imu_osc = OscHelper("imu", args.imu_board_ip, args.imu_board_send_port, args.imu_board_receive_port)
bridge_osc = OscHelper("bridge", "127.0.0.1", args.bridge_send_port, args.bridge_receive_port)

imu_osc.map("/quat", receive_quaternion)
main_osc.map("/motor/1/ticks", receive_speed_ticks)
bridge_osc.map("/morphoses/action", receive_action)
bridge_osc.map("/morphoses/next", receive_next)
bridge_osc.map("/morphoses/begin", receive_begin)
bridge_osc.map("/morphoses/end", receive_end)
bridge_osc.map("/morphoses/rgb", receive_rgb)

def interrupt(signup, frame):
    global client, server
    print("Exiting program...")
    osc_terminate()
    sys.exit()

signal.signal(signal.SIGINT, interrupt)

# Serve forever.
while True:
    osc_process()
