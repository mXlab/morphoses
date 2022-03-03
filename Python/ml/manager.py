from agent import Agent

import logging
import threading

logging.basicConfig(format="%(asctime)s: %(message)s", level=logging.INFO,
                    datefmt="%H:%M:%S")

# This class implements the RL loop by bringing together the world and the agents. It also manages the different control
# processes involved, such as the ability to switch between different behaviors, etc.
class Manager:
    def __init__(self, world, settings):
        self.world = world

        self.behaviors = settings['behaviors']
        self.robots = settings['robots']

        self.agents = {}
        self.current_agents = {}

        for robot in self.robots:
            self.agents[robot] = {}
            self.current_agents[robot] = self.create_agent(robot, self.robots[robot])

    def agent_exists(self, robot_name, behavior_name):
        return (robot_name in self.agents) and (behavior_name in self.agents[robot_name])

    def get_agent(self, robot_name, behavior_name):
        if self.agent_exists(robot_name, behavior_name):
            return self.agents[robot_name][behavior_name]
        else:
            return self.create_agent(robot_name, behavior_name)

    def create_agent(self, robot_name, behavior_name):
        agent = Agent(world=self.world, name=robot_name, **self.behaviors[behavior_name])
        self.agents[robot_name][behavior_name] = agent
        return agent

    def get_current_agent(self, robot_name):
        return self.current_agents(robot_name)

    def set_current_agent(self, robot_name, behavior_name):
        self.current_agents[robot_name] = self.get_agent(robot_name, behavior_name)

    def get_current_agents(self):
        return self.current_agents

    def begin(self):
        print("** Begin **")
        self.world.begin()

        for a in self.current_agents.values():
            print("** Begin agent **")
            a.begin()

        self.world.sleep(1)
        print("** Update world **")
        self.world.update()

    def step(self):
        print("** World step **")
        self.world.step()

        print("** Agents steps **")

        threads = list()
        for a in self.current_agents.values():
            step_thread = threading.Thread(target=self.run_step_agent, args=(a,))
            threads.append(step_thread)
            step_thread.start()

        for index, thread in enumerate(threads):
            logging.info("Main    : before joining thread %d.", index)
            thread.join()
            logging.info("Main    : thread %d done", index)

    # Callback for threads.
    def run_step_agent(self, agent):
        logging.info("Stepping agent {}".format(agent.get_name()))
        agent.step()
        logging.info("End step agent {}".format(agent.get_name()))
