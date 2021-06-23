import argparse
import yaml
from agent import Agent
from world import World

if __name__ == "__main__":
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
    agent = Agent(world=world, **run_settings)

    agent.begin()

    while True:
        world.step()
        agent.step()