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
    def __init__(self, broker, port, world, settings):
        self.world = world
        
        # Create client.
        self.client = mqtt.Client()
        self.broker = broker
        self.port = port
        self.qos = settings.get('qos', 1)

        self.subscriptions = {}  # Path: callback mapping
        print(self.broker, self.port, self.qos)


    # The callback for when the client receives a CONNACK response from the server.
    def _onConnect(self, client, args, flags, rc):
        print("Connected with result code "+str(rc))
        for topic in self.subscriptions.keys():
            client.subscribe(topic)

    # The callback for when a PUBLISH message is received from the server.
    def _onMessage(self, client, userdata, msg):
        # print(f"Received message on {msg.topic}: {msg.payload.decode()}")
        if msg.topic in self.subscriptions:
            callback, args = self.subscriptions[msg.topic]
            try:
                payload = json.loads(msg.payload)  # Attempt to parse JSON payload
            except json.JSONDecodeError:
                payload = msg.payload.decode()  # Use raw string if not JSON
            callback(payload, args)

    def begin(self):
        # Assign client callbacks.
        self.client.on_connect = self._onConnect
        self.client.on_message = self._onMessage
        # Connect to broker.
        self.client.connect(self.broker, self.port)
        # Start threaded loop.
        self.client.loop_start()

    def terminate(self):
        # Unsubscribe from all topics.
        self.client.loop_stop()

    def send(self, path, args=None):
        message = self.json_encode(args) if isinstance(args, (dict, list)) else str(args) if args is not None else ''
        self.client.publish(path, message, qos=self.qos)

    def map(self, path, callback, args=None):
        self.subscriptions[path] = (callback, args)
        self.client.subscribe(path, qos=self.qos)

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

class Messaging:
    def __init__(self, world, settings):
        self.manager = None
        self.world = world

        # Map RTLS nodes to names.
        self.rtls_nodes = {}
        for entity in settings['robots'] + settings['things']:
            self.rtls_nodes[entity['rtls_id']] = entity['name']

        # Init MQTT.
        try:
            self.mqtt = MqttHelper(settings['rtls_gateway']['ip'], settings['rtls_gateway']['rtls_recv_port'], world, settings)
        except Exception:
            print("Problem with starting MQTT, please check server.")
            sys.exit()

        # Init MQTT.
        self.mqtt.begin()

        # Subscribe to topics.

        # Subscribe to RTLS location updates.
        for node_id in self.rtls_nodes.keys():
            self.mqtt.map("dwm/node/{}/uplink/location".format(node_id), self.receive_location, node_id)
        
        # Subscribe to robot-specific topics.
        for robot in settings['robots']:
            name = robot['name']
            self.mqtt.map("morphoses/{}/data".format(name), self.receive_data, name)
            self.mqtt.map("morphoses/debug", self.receive_debug, (name, "debug"))
            self.mqtt.map("morphoses/error", self.receive_debug, (name, "error"))

    def set_manager(self, manager):
        self.manager = manager

    def send(self, robot_name, address, args=[]):
        print("Sending {} {} {}".format(robot_name, address, str(args)))
        self.mqtt.send("morphoses/{}{}".format(robot_name, address), args)

    def send_info(self, robot_name, address, args=[]):
        self.send(robot_name, "/info{}".format(address), args)

    def send_debug(self, message):
        self.mqtt.send("morphoses/debug", message)

    def loop(self):
        pass

    def begin(self):
        print("Messaging begin")
        self.mqtt.begin()

    def terminate(self):
        print('Messaging terminated')
        self.mqtt.terminate()

    # Callbacks. ################################################################
        
    # Receive RTLS location.
    def receive_location(self, data, node_id):
        pos = data['position']
        if pos['quality'] > 40 and node_id in self.rtls_nodes.keys():
            self.world.store_position(self.rtls_nodes[node_id], [float(pos['x']), float(pos['y'])])

    # Receive data from robot.
    def receive_data(self, data, name):
        print("*** Receiving data Mqtt callback***")
        self.world.store_rotation_data_main(name, 
                                            data['main']['quat'] + data['main']['d-quat'] +
                                            data['main']['rot'] + data['main']['d-rot'])
        self.world.store_rotation_data_side(name, 
                                            data['side']['quat'] + data['side']['d-quat'] +
                                            data['side']['rot'] + data['side']['d-rot'])

    # def receive_accuracy(self, accuracy, args):
    #     name, imu_is_main = args
    #     imu = 'main' if imu_is_main else 'side'
    #     self.world.send_info(name, "/{}/accur".format(imu), accuracy)

    # def receive_battery(self, battery, name):
    #     self.world.send_info(name, "/battery", battery)

    def receive_debug(self, message, args):
        name, type = args
        print(">>>>>>[{}] *** !!! {}: [{}] !!! ***".format(name, type, message))

    def set_behavior(self, args, extra):
        robot_name, behavior_name = args
        print("*** CHANGING BEHAVIOR: {} => {}".format(robot_name, behavior_name))
        self.manager.set_current_agent(robot_name, behavior_name)

    def send_animation(self, robot_name, animation):
        # animation = {
        #     "base": [255, 255, 255, 255],
        #     "alt": [0, 0, 0, 0],
        #     "period": 4,
        #     "noise": 0.1,
        #     "region": 0,
        #     "type": 0
        # }
        self.send(robot_name, "/animation", animation)

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

