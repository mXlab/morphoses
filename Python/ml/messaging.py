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
    def __init__(self, name, ip, send_port, receive_port):
        print("Creating OSC link at IP {} send = {} recv = {}".format(ip, send_port, receive_port))
        self.name = name
        osc_udp_client(ip, int(send_port), self.client_name())
        osc_udp_server("0.0.0.0", int(receive_port), self.server_name())

    def client_name(self):
        return self.name + "_client"

    def server_name(self):
        return self.name + "_server"

    def map(self, path, function, extra=None):
        osc_method(path, function, argscheme=osm.OSCARG_EXTRA + osm.OSCARG_DATA, extra=extra)

    def send_message(self, path, args):
        if not isinstance(args, list):
            args = [ args ]
        msg = oscbuildparse.OSCMessage(path, None, args)
        osc_send(msg, self.client_name())
        osc_process()

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

        self.osc_robots = []
        for robot in settings['robots']:
            item = {}
            name = item['name'] = robot['name']
            main = robot['main']
            imu  = robot['imu']
            item['main'] = OscHelper(name + "-main", main['ip'], main['osc_send_port'], main['osc_recv_port'])
            item['imu']  = OscHelper(name + "-imu",   imu['ip'],  imu['osc_send_port'],  imu['osc_recv_port'])

            item['main'].map("/quat", self.receive_quaternion_main, name)
            item['imu'] .map("/quat", self.receive_quaternion,      name)

            self.osc_robots.append(item)

        # Init MQTT.
        try:
            self.mqtt = MqttHelper(settings['rtls_gateway']['ip'], settings['rtls_gateway']['rtls_recv_port'], world, settings)
        except Exception:
            print("Problem with starting MQTT, please check server.")

    def loop(self):
        osc_process()
        self.mqtt.loop()

    def terminate(self):
        osc_terminate()

    def receive_quaternion(self, name, quat):
        self.world.store_quaternion(name, quat)

    def receive_quaternion_main(self, name, quat):
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
