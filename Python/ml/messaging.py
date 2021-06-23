import argparse
import time
import signal
import sys

from osc4py3.as_eventloop import *
from osc4py3 import oscbuildparse
from osc4py3 import oscmethod as osm

import paho.mqtt.client as mqtt
import json

class OscHelper:
    def __init__(self, name, ip, send_port, recv_port):
        print("Creating OSC link at IP {} send = {} recv = {}".format(ip, send_port, recv_port))
        self.name = name
        self.ip = ip
        self.send_port = send_port
        self.recv_port = recv_port
        osc_udp_client(ip, int(send_port), self.client_name())
        osc_udp_server("0.0.0.0", int(recv_port), self.server_name())
        self.maps = {}

    def client_name(self):
        return self.name + "_client"

    def server_name(self):
        return self.name + "_server"

    def send_message(self, path, args):
        if not isinstance(args, list):
            args = [ args ]
        msg = oscbuildparse.OSCMessage(path, None, args)
        osc_send(msg, self.client_name())
        osc_process()

    # Adds an OSC path by assigning it to a function, with optional extra data.
    def map(self, path, function, extra=None):
        self.maps[path] = { 'function': function, 'extra': extra }

    # Dispatches OSC message to appropriate function, if it corresponds to helper.
    def dispatch(self, address, ip, data):
        # Check if address matches and if IP corresponds: if so, call mapped function.
        if address in self.maps and ip == self.ip:
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

    def loop(self):
        self.mqtt_client.loop()

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
        print(msg.topic+" "+str(msg.payload))
        data = json.loads(msg.payload)
        pos = data['position']
        if (pos['quality'] > 0): # only update if quality is good
            node_id = msg.topic[9:13]
            if node_id in self.rtls_nodes.keys():
                self.world.store_position(self.rtls_nodes[node_id], [float(pos['x']), float(pos['y'])])

class Messaging:
    def __init__(self, world, settings):
        self.world = world

        # Init OSC.
        osc_startup()

        # Send all paths to the dispatch() method with information.
        osc_method("*", self.dispatch, argscheme=osm.OSCARG_ADDRESS + osm.OSCARG_SRCIDENT + osm.OSCARG_DATA)

        # Create array of OscHelper objects for communicating with the robots.
        self.osc_robots = []
        for robot in settings['robots']:
            name = robot['name']
            main = robot['main']
            imu  = robot['imu']
            osc_main = OscHelper(name + "-main", main['ip'], main['osc_send_port'], main['osc_recv_port'])
            osc_imu  = OscHelper(name + "-imu",   imu['ip'],  imu['osc_send_port'],  imu['osc_recv_port'])

            osc_main.map("/quat", self.receive_quaternion_main, name)
            osc_imu .map("/quat", self.receive_quaternion,      name)

            self.osc_robots.append(osc_main)
            self.osc_robots.append(osc_imu)

        # Init MQTT.
        try:
            self.mqtt = MqttHelper(settings['rtls_gateway']['ip'], settings['rtls_gateway']['rtls_recv_port'], world, settings)
        except Exception:
            print("Problem with starting MQTT, please check server.")
            sys.exit()

    def dispatch(self, address, src_info, data):
        ip = src_info[0]
        for osc in self.osc_robots:
            osc.dispatch(address, ip, data)

    def loop(self):
        osc_process()
        self.mqtt.loop()

    def terminate(self):
        osc_terminate()

    def receive_quaternion(self, quat, name):
        self.world.store_quaternion(name, quat)

    def receive_quaternion_main(self, quat, name):
        self.world.store_quaternion_main(name, quat)

def interrupt(signup, frame):
    global messaging
    print("Exiting program...")
    messaging.terminate()
    sys.exit()

signal.signal(signal.SIGINT, interrupt)

if __name__ == '__main__':
    import yaml
    import world
    settings = yaml.load(open('settings.yml', 'r'), Loader=yaml.SafeLoader)
    w = world.World(settings)
    messaging = Messaging(w, settings)
    while True:
        messaging.loop()
        w.update()
        w.debug()
#        time.sleep(1)