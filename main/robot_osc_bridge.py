import argparse
import time
import signal
import sys

from osc4py3.as_eventloop import *
from osc4py3 import oscbuildparse
from osc4py3 import oscmethod as osm

import paho.mqtt.client as mqtt
import json

next_data_requested = False
start_time = time.time()
current_speed = 0
current_steer = 0
current_position = [0, 0]
current_tag_position = [0, 0]
current_quaternion = [0, 0, 0, 0]
current_n_revolutions = 0
accumulated_sound_level = 0.

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
    print("--> Result: speed={} steer={}".format(speed, steer))
    main_osc.send_message("/speed", speed)
    main_osc.send_message("/steer", steer)

# Receive peak sound level from external microphone.
def receive_peak_sound_level(unused_addr, current_sound_level):
    global accumulated_sound_level

    # Add current sound level to accumulated sound level.
    accumulated_sound_level += current_sound_level

    print("Received sound level: {}".format([current_sound_level, accumulated_sound_level]))

# Reset accumulated sound level.
def reset_accumulated_sound_level():
    global accumulated_sound_level
    accumulated_sound_level = 0.

# Main program asks for next data point.
def receive_next(unused_addr):
    global next_data_requested, bridge_osc
    print("Requested next data")
    send_data()

def receive_begin(unused_addr):
    global next_data_requested
    main_osc.send_message("/power", 1)
    print("Requested begin")
    send_data()

def receive_end(unused_addr):
    global next_data_requested
    print("Requested next data")
    main_osc.send_message("/power", 0)
    # Switch motors off
    main_osc.send_message("/speed", 0)
    main_osc.send_message("/steer", 0)

def receive_rgb(unused_addr, r, g, b):
    global next_data_requested
    print("RGB: {} {} {}".format(r, g, b))
    main_osc.send_message("/rgb", r, g, b)

def send_data():
    global current_tag_position, current_position, current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested, accumulated_sound_level
    bridge_osc.send_message("/morphoses/data", [ 0, time.time() - start_time ] + current_position + current_quaternion + [current_n_revolutions, current_speed, current_steer ] + current_tag_position + [accumulated_sound_level])   

    # Reset accumulated sound level each time data is sent to the RL script.
    reset_accumulated_sound_level()

# Preserve state value for next call.
def receive_quaternion(unused_addr, q0, q1, q2, q3):
    global current_position, current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested
    current_quaternion = [q0, q1, q2, q3]
    print("Received quaternion: {}".format([q0, q1, q2, q3]))

# Preserve state value for next call.
def receive_speed_ticks(unused_addr, ticks):
    global current_position, current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested
    current_n_revolutions = ticks / N_MOTOR1_TICKS_PER_REVOLUTION
    print("Received speed ticks: {}".format([ticks, current_n_revolutions]))

# The callback for when the client receives a CONNACK response from the server.
def mqtt_on_connect(client, args, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
#    client.subscribe("$SYS/#")
    client.subscribe("dwm/node/{}/uplink/location".format(args.rtls_robot_node_id))
    client.subscribe("dwm/node/{}/uplink/location".format(args.rtls_thing_node_id))

# The callback for when a PUBLISH message is received from the server.
def mqtt_on_message(client, userdata, msg):
    global current_tag_position, current_position, current_quaternion, current_n_revolutions, current_timestamp, start_time, next_data_requested
    print(msg.topic+" "+str(msg.payload))
    data = json.loads(msg.payload)
    pos = data['position']
    if (pos['quality'] > 0): # only update if quality is good

        # If robot ID: update current robot position
        if msg.topic[9:13] == args.rtls_robot_node_id:
            current_position = [float(pos['x']), float(pos['y'])]

        # If thing ID: update current robot position
        elif msg.topic[9:13] == args.rtls_thing_node_id:
            current_tag_position = [float(pos['x']), float(pos['y'])]

# Create parser
parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument("--bridge-receive-port", default="8000",
                        help="Specify the port number where data is received from the main application.")
parser.add_argument("--bridge-send-port", default="8100",
                        help="Specify the port number to send to the main application.")

parser.add_argument("--main-board-ip", default="192.168.0.110",
                        help="Specify the ip address of the main board.")
parser.add_argument("--main-board-send-port", default="8000",
                        help="Specify the port number to send to main board.")
parser.add_argument("--main-board-receive-port", default="8110",
                        help="Specify the port number where data is received from the main board.")

parser.add_argument("--imu-board-ip", default="192.168.0.111",
                        help="Specify the ip address of the IMU board.")
parser.add_argument("--imu-board-send-port", default="8000",
                        help="Specify the port number to send to IMU board.")
parser.add_argument("--imu-board-receive-port", default="8111",
                        help="Specify the port number where data is received from the IMU board.")

parser.add_argument("--rtls-gateway-ip", default="192.168.0.200",
                        help="Specify the ip address of the RTLS Rasbperry Pi gateway.")
parser.add_argument("--rtls-gateway-port", default=1883,
                        help="Specify the port number of the RLTS Raspberry Pi gateway MQTT.")
parser.add_argument("--rtls-robot-node-id", default="1a1e",
                        help="The Node ID of the robot in the RTLS network.")
parser.add_argument("--rtls-thing-node-id", default="5a8e",
                        help="The Node ID of the thing tag in the RTLS network.")

args = parser.parse_args()

osc_startup()

main_osc = OscHelper("main", args.main_board_ip, args.main_board_send_port, args.main_board_receive_port)
imu_osc = OscHelper("imu", args.imu_board_ip, args.imu_board_send_port, args.imu_board_receive_port)
bridge_osc = OscHelper("bridge", "127.0.0.1", args.bridge_send_port, args.bridge_receive_port)

imu_osc.map("/quat", receive_quaternion)
#main_osc.map("/motor/1/ticks", receive_speed_ticks)
bridge_osc.map("/morphoses/action", receive_action)
bridge_osc.map("/morphoses/next", receive_next)
bridge_osc.map("/morphoses/begin", receive_begin)
bridge_osc.map("/morphoses/end", receive_end)
bridge_osc.map("/morphoses/rgb", receive_rgb)
bridge_osc.map("/morphoses/sound", receive_peak_sound_level)

def interrupt(signup, frame):
    global client, server
    print("Exiting program...")
    osc_terminate()
    sys.exit()

signal.signal(signal.SIGINT, interrupt)

mqtt_client = mqtt.Client(userdata=args)
mqtt_client.on_connect = mqtt_on_connect
mqtt_client.on_message = mqtt_on_message

mqtt_client.connect(args.rtls_gateway_ip, args.rtls_gateway_port)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
#mqtt_client.loop_forever()

# Serve forever.
while True:
 #   print("Iterate")
    osc_process()
    mqtt_client.loop()
