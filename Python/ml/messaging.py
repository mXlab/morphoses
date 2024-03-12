import argparse
import time
import signal
import sys

from osc4py3.as_eventloop import *
from osc4py3 import oscbuildparse
from osc4py3 import oscmethod as osm

from pythonosc import osc_message_builder, osc_bundle_builder
from pythonosc import udp_client


import paho.mqtt.client as mqtt
import json

class OscHelper:
    def __init__(self, name, ip, send_port, recv_port, redirect_ip=None, redirect_port=8001):
        print("Creating OSC link at IP {} send = {} recv = {}".format(ip, send_port, recv_port))
        self.name = name
        self.ip = ip
        self.send_port = send_port
        self.recv_port = recv_port
        self.client = udp_client.SimpleUDPClient(ip, int(send_port))
        if redirect_ip is not None:
            self.client_redirect = udp_client.SimpleUDPClient(redirect_ip, int(redirect_port))
        else:
            self.client_redirect = None
        osc_udp_server("0.0.0.0", int(recv_port), self.server_name())
        self.maps = {}

    def client_name(self):
        return self.name + "_client"

    def server_name(self):
        return self.name + "_server"

    def send_message(self, path, args):
        if not isinstance(args, list):
            args = [ args ]
        self.client.send_message(path, args)
        # print("Sending message {} {} to {}".format(path, str(args), self.client_name()))

    # Send group of messages as a bundle.
    def send_bundle(self, messages):
        bundle = osc_bundle_builder.OscBundleBuilder(osc_bundle_builder.IMMEDIATELY)
        for path, args in messages.items():
            print("Sending bundle with {}".format(path))
            if not isinstance(args, list):
                args = [ args ]
            msg_builder = osc_message_builder.OscMessageBuilder(address=path)
            for a in args:
                msg_builder.add_arg(a)
            bundle.add_content(msg_builder.build())
        self.client.send(bundle.build())

    # Adds an OSC path by assigning it to a function, with optional extra data.
    def map(self, path, function, extra=None):
        self.maps[path] = { 'function': function, 'extra': extra }

    # Dispatches OSC message to appropriate function, if it corresponds to helper.
    def dispatch(self, address, ip, data):
        # Redirect
        if self.client_redirect is not None:
            self.client_redirect.send_message(address, data)
        # Check if address matches and if IP corresponds: if so, call mapped function.
        if address in self.maps and (ip == self.ip or (ip == '127.0.0.1' and self.ip == 'localhost')):
            item = self.maps[address]
            func = item['function']
            if item['extra'] is None:
                func(data)
            else:
                func(data, item['extra'])

class MqttHelper:
    def __init__(self, ip, port, world, settings):
        self.world = world
        # Create client.
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.mqtt_on_connect
        self.mqtt_client.on_message = self.mqtt_on_message

        # Start client.
        self.mqtt_client.connect(ip, port)

        # Collect basic connection information.
        self.rtls_nodes = {}
        for entity in settings['robots'] + settings['things']:
            self.rtls_nodes[entity['rtls_id']] = entity['name']

    # The callback for when the client receives a CONNACK response from the server.
    def mqtt_on_connect(self, client, args, flags, rc):
        print("Connected with result code "+str(rc))

        # Subscribing in on_connect() means that if we lose the connection and
        # reconnect then subscriptions will be renewed.
    #    client.subscribe("$SYS/#")
        for node_id in self.rtls_nodes.keys():
            client.subscribe("dwm/node/{}/uplink/location".format(node_id))

    # The callback for when a PUBLISH message is received from the server.
    def mqtt_on_message(self, client, userdata, msg):
        # print(msg.topic+" "+str(msg.payload))
        data = json.loads(msg.payload)
        pos = data['position']
        if (pos['quality'] > 50): # only update if quality is good
            node_id = msg.topic[9:13]
            if node_id in self.rtls_nodes.keys():
                self.world.store_position(self.rtls_nodes[node_id], [float(pos['x']), float(pos['y'])])

    def begin(self):
        # Start threaded loop.
        self.mqtt_client.loop_start()

    def terminate(self):
        self.mqtt_client.loop_stop()

    def round_floats(self, o):
        if isinstance(o, float):
            return round(o, 5)
        if isinstance(o, dict):
            return {k: self.round_floats(v) for k, v in o.items()}
        if isinstance(o, (list, tuple)):
            return [self.round_floats(x) for x in o]
        return o

    def json_encode(self, data):
        return json.dumps(self.round_floats(data))

    def publish_animation(self, robot_name, animation):
        self.mqtt_client.publish("morphoses/{}/animation".format(robot_name), self.json_encode(animation), 1)

class Messaging:
    def __init__(self, world, settings):
        self.manager = None
        self.world = world

        # Init OSC.
        osc_startup()

        # Send all paths to the dispatch() method with information.
        osc_method("*", self.dispatch, argscheme=osm.OSCARG_ADDRESS + osm.OSCARG_SRCIDENT + osm.OSCARG_DATA)

        # Create array of OscHelper objects for communicating with the robots.
        self.osc_robots = {}
        for robot in settings['robots']:
            name = robot['name']
            osc_helper = OscHelper(name, robot['ip'], robot['osc_send_port'], robot['osc_recv_port'],
                                   settings['visualizer']['ip'], robot['osc_visualizer_send_port'])

            osc_helper.map("/main/quat",  self.receive_quaternion, (name, True))
            osc_helper.map("/side/quat",  self.receive_quaternion, (name, False))
            osc_helper.map("/main/data",  self.receive_rotation_data, (name, True))
            osc_helper.map("/side/data",  self.receive_rotation_data, (name, False))
            osc_helper.map("/main/accur", self.receive_accuracy,   (name, True))
            osc_helper.map("/side/accur", self.receive_accuracy,   (name, False))
            osc_helper.map("/battery",    self.receive_battery,     name)
            osc_helper.map("/debug",      self.receive_debug,       (name, "debug"))
            osc_helper.map("/error",      self.receive_debug,       (name, "error"))
            osc_helper.map("/ready",      self.receive_debug,       (name, "ready"))

            self.osc_robots[name] = osc_helper

        # Local info logging client.
        # self.info_client = udp_client.SimpleUDPClient("localhost", 8001)
        self.info_client = udp_client.SimpleUDPClient("192.168.0.255", 8001, allow_broadcast=True)

        control_interface_settings = settings['control_interface']
        self.osc_control_interface = OscHelper("control-interface", control_interface_settings['ip'], control_interface_settings['osc_send_port'], control_interface_settings['osc_recv_port'])
        self.osc_control_interface.map("/morphoses/set-behavior", self.set_behavior, "")

        # Init MQTT.
        try:
            self.mqtt = MqttHelper(settings['rtls_gateway']['ip'], settings['rtls_gateway']['rtls_recv_port'], world, settings)
        except Exception:
            print("Problem with starting MQTT, please check server.")
            sys.exit()

    def set_manager(self, manager):
        self.manager = manager

    def send(self, robot_name, address, args=[]):
        self.osc_robots[robot_name].send_message(address, args)

    def send_bundle(self, robot_name, messages):
        self.osc_robots[robot_name].send_bundle(messages)

    def send_info(self, address, args=[]):
        self.info_client.send_message(address, args)

    def dispatch(self, address, src_info, data):
        ip = src_info[0]
        for node in self.osc_robots.values():
            node.dispatch(address, ip, data)

        self.osc_control_interface.dispatch(address, ip, data)

    def loop(self):
        osc_process()

    def begin(self):
        self.mqtt.begin()

    def terminate(self):
        osc_terminate()
        self.mqtt.terminate()

    def receive_rotation_data(self, data, args):
        name, imu_is_main = args
        if imu_is_main:
            self.world.store_rotation_data_main(name, data)
        else:
            self.world.store_rotation_data_side(name, data)

    def receive_quaternion(self, quat, args):
        name, imu_is_main = args
        if imu_is_main:
            self.world.store_quaternion_main(name, quat)
        else:
            self.world.store_quaternion_side(name, quat)

    def receive_accuracy(self, accuracy, args):
        name, imu_is_main = args
        imu = 'main' if imu_is_main else 'side'
        self.world.send_info(name, "/{}/accur".format(imu), accuracy)

    def receive_battery(self, battery, name):
        self.world.send_info(name, "/battery", battery)

    def receive_debug(self, message, args):
        name, type = args
        print(">>>>>>[{}] *** !!! {}: [{}] !!! ***".format(name, type, message))

    def set_behavior(self, args, extra):
        robot_name, behavior_name = args
        print("*** CHANGING BEHAVIOR: {} => {}".format(robot_name, behavior_name))
        self.manager.set_current_agent(robot_name, behavior_name)

    def send_animation(self, robot_name, animation):
        animation = {
            "base": [255, 255, 255, 255],
            "alt": [0, 0, 0, 0],
            "period": 4,
            "noise": 0.1,
            "region": 0,
            "type": 0
        }
        self.mqtt.publish_animation(robot_name, animation)

def interrupt(signup, frame):
    global my_world, stop
    print("Exiting program...")
    my_world.terminate()
    stop = True
    sys.exit()


if __name__ == '__main__':
    import yaml
    import world

    signal.signal(signal.SIGINT, interrupt)

    stop = False
    settings = yaml.load(open('settings.yml', 'r'), Loader=yaml.SafeLoader)
    my_world = world.World(settings)
    while not stop:
        my_world.step()
        my_world.debug()
#        time.sleep(0.5)

