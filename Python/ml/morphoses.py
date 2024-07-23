import argparse
import yaml
import signal
import sys
import time

import threading

from agent import Agent
from world import World
from manager import Manager

def terminate(signup, frame):
    global world
    print("Exiting program...")
    world.terminate()
    sys.exit()

if __name__ == "__main__":

    signal.signal(signal.SIGINT,  terminate)
    signal.signal(signal.SIGTERM, terminate)

    # Create parser
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("run_file", type=str, help="File containing the information to run the robots")
    parser.add_argument("--settings-file", type=str, default="settings.yml", help="File containing the common settings")

    parser.add_argument("--override-robots", type=str, default=None, help="(optional) Overrides list of robots")

    # Parse arguments.
    args = parser.parse_args()

    # Display title.
    print(r"""
          
  __  __                  _                        
 |  \/  |                | |                       
 | \  / | ___  _ __ _ __ | |__   ___  ___  ___ ___ 
 | |\/| |/ _ \| '__| '_ \| '_ \ / _ \/ __|/ _ / __|
 | |  | | (_) | |  | |_) | | | | (_) \__ |  __\__ \
 |_|  |_|\___/|_|  | .__/|_| |_|\___/|___/\___|___/
                   | |                             
                   |_|                                       
                    
    """)

    run_settings = yaml.load(open(args.run_file, 'r'), Loader=yaml.SafeLoader)
    settings = yaml.load(open(args.settings_file, 'r'), Loader=yaml.SafeLoader)

    # Override robots.
    if args.override_robots is not None:
        run_settings['robots'] = args.override_robots.split(',')

    # Create world.
    world = World(settings)

    # Create agents (for now just one agent).
    behaviors = run_settings['behaviors']
    robots = run_settings['robots']
    
    manager = Manager(world, run_settings)
    world.set_manager(manager)

    manager.begin()

    while manager.is_running():
        manager.step()

    manager.end()

    world.terminate()
    print("Done")
