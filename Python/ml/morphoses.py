import argparse
import yaml
import signal
import sys
import time

import threading

from agent import Agent
from world import World

def interrupt(signup, frame):
    global world
    print("Exiting program...")
    world.terminate()
    sys.exit()

import logging

format = "%(asctime)s: %(message)s"
logging.basicConfig(format=format, level=logging.INFO,
                    datefmt="%H:%M:%S")


def step_agent(agent):
    logging.info("Stepping agent {}".format(agent.get_name()))
    agent.step()
    logging.info("End step agent {}".format(agent.get_name()))

if __name__ == "__main__":

    signal.signal(signal.SIGINT, interrupt)

    # Create parser
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("run_file", type=str, help="File containing the information to run the robots")
    parser.add_argument("--settings-file", type=str, default="settings.yml", help="File containing the common settings")

    # Parse arguments.
    args = parser.parse_args()

    run_settings = yaml.load(open(args.run_file, 'r'), Loader=yaml.SafeLoader)
    settings = yaml.load(open(args.settings_file, 'r'), Loader=yaml.SafeLoader)

    # Create world.
    world = World(settings)

    # Create agents (for now just one agent).
    behaviors = run_settings['behaviors']
    robots = run_settings['robots']
    agents = []
    for robot in robots:
        b = robots[robot]
        agents.append(Agent(world=world, name=robot, **behaviors[b]))

    world.begin()
    for a in agents:
        a.begin()

    world.sleep(1)
    world.update()

    while True:
        world.step()
        for a in agents:
            a.step()
