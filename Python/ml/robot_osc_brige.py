import argparse
import time
import threading

from pythonosc import dispatcher
from pythonosc import osc_server
from pythonosc import osc_message_builder
from pythonosc import udp_client

next_data_requested = False
start_time = time.time()
current_speed = 0
current_steer = 0

class OscHelper:
    def __init__(self, ip, send_port, receive_port):
        print("Creating OSC link at IP {} send = {} recv = {}".format(ip, send_port, receive_port))
        self.dispatcher = dispatcher.Dispatcher()
        self.server = osc_server.ThreadingOSCUDPServer(("localhost", int(receive_port)), self.dispatcher)
        self.client = udp_client.SimpleUDPClient(ip, int(send_port))
        self.server_thread = threading.Thread(target=self.server.serve_forever)

    def map(self, path, function):
        self.dispatcher.map(path, function)

    def send_message(self, path, args):
        self.client.send_message(path, args)

    def start(self):
        self.server_thread.start()

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
    next_data_requested = False

def receive_rgb(unused_addr, r, g, b):
    global next_data_requested
    print("RGB: {} {} {}".format(r, g, b))
    main_osc.send_message("/red", int(r))
    main_osc.send_message("/green", int(g))
    main_osc.send_message("/blue", int(b))

# Preserve state value for next call.
def receive_quaternion(unused_addr, q0, q1, q2, q3):
    global current_quaternion, current_timestamp, start_time, next_data_requested
#    print("Received quaternion: {}".format([q0, q1, q2, q3]))
    if next_data_requested:
        bridge_osc.send_message("/morphoses/data", [ 0, time.time() - start_time, 0, 0, q0, q1, q2, q3, current_speed, current_steer ])
        next_data_requested = False

# Create parser
parser = argparse.ArgumentParser()

parser.add_argument("--bridge-receive-port", default="7765",
                        help="Specify the port number where data is received from the main application.")
parser.add_argument("--bridge-send-port", default="7767",
                        help="Specify the port number to send to the main application.")

parser.add_argument("--main-board-ip", default="192.168.0.101",
                        help="Specify the ip address of the main board.")
parser.add_argument("--main-board-send-port", default="8765",
                        help="Specify the port number to send to main board.")
parser.add_argument("--main-board-receive-port", default="8766",
                        help="Specify the port number where data is received from the main board.")

parser.add_argument("--imu-board-ip", default="192.168.0.102",
                        help="Specify the ip address of the IMU board.")
parser.add_argument("--imu-board-send-port", default="8765",
                        help="Specify the port number to send to IMU board.")
parser.add_argument("--imu-board-receive-port", default="8767",
                        help="Specify the port number where data is received from the IMU board.")

args = parser.parse_args()

main_osc = OscHelper(args.main_board_ip, args.main_board_send_port, args.main_board_receive_port)
imu_osc = OscHelper(args.imu_board_ip, args.imu_board_send_port, args.imu_board_receive_port)
bridge_osc = OscHelper("127.0.0.1", args.bridge_send_port, args.bridge_receive_port)

imu_osc.map("/quat", receive_quaternion)
bridge_osc.map("/morphoses/action", receive_action)
bridge_osc.map("/morphoses/next", receive_next)
bridge_osc.map("/morphoses/begin", receive_begin)
bridge_osc.map("/morphoses/end", receive_end)
bridge_osc.map("/morphoses/rgb", receive_rgb)

main_osc.start()
imu_osc.start()
bridge_osc.start()
