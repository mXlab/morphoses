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

        self.sequence = settings['sequence']
        self.sequence_current = 0
        self.sequence_repeat = settings['settings']['repeat_sequence']
        self.running = True

        self.agents = {}
        self.current_agents = {}

        # Create all agents.
        for robot in self.robots:
            self.agents[robot] = {}
            for behavior in self.sequence:
                self.create_agent(robot, behavior)

        self.sequence_set_current(0, begin=False)

    def agent_exists(self, robot_name, behavior_name):
        return (robot_name in self.agents) and (behavior_name in self.agents[robot_name])

    def sequence_set_current(self, index, begin=True, reset=False):
        self.sequence_current = index % len(self.sequence)
        behavior = self.sequence[self.sequence_current]
        for robot in self.robots:
            self.set_current_agent(robot, behavior, begin, reset)

    def sequence_has_next(self):
        return self.sequence_repeat or (self.sequence_current + 1) < len(self.sequence)

    def sequence_next(self, begin=True, reset=False):
        self.sequence_set_current(self.sequence_current + 1, begin, reset)

    def sequence_current_behavior_name(self):
        return self.sequence[self.sequence_current]
    
    def sequence_current_behavior(self):
        return self.behaviors[self.sequence_current_behavior_name()]

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

    def set_current_agent(self, robot_name, behavior_name, begin=True, reset=False):
        agent = self.get_agent(robot_name, behavior_name)
        if begin:
            agent.begin()
        if reset:
            agent.reset()
        self.current_agents[robot_name] = agent

    def get_current_agents(self):
        return self.current_agents
    
    def behavior_begin(self):
        print("Calling function behavior_begin")
        title = self.sequence_current_behavior()['title']
        self.world.send_info("all", "/begin", title)
        for a in self.current_agents.values():
            print("** Begin agent {} **".format(a.get_name()))
            a.begin()
        
        self.world.sleep(10)

    def behavior_end(self):
        title = self.sequence_current_behavior()['title']
        # self.world.send_info("all", "/end", title)
    
    def is_running(self):
        return self.running
    
    def begin(self):
        print("** Begin **")
        self.world.begin()

        # Begin all agents.
        self.behavior_begin()

        print("** Update world **")
        self.world.update()

    def step(self):
        # Step world.
        print("** World step **")
        self.world.step()

        # Step all agents.
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

        # Check if all agents are stopped.
        all_stopped = True
        for a in self.current_agents.values(): # Verify if stopped.
            if not a.is_stopped():
                all_stopped = False
                break

        # If all agents are stopped, move to next behavior.
        if all_stopped:
            print("*** ALL AGENTS STOPPED ***")
            self.behavior_end()
            self.world.sleep(10) # Wait

            print("*** Done waiting move to next behavior ***")
            # Fade out.
            for a in self.current_agents.values():
                self.world.display_idle(a) # Display idle mode.
            self.world.sleep(5) # Wait

            # Change behavior.
            if self.sequence_has_next():
                self.sequence_next(begin=False, reset=False)
                self.behavior_begin()
                self.world.sleep(1) # Wait
            else:
                self.running = False

    # Callback for threads.
    def run_step_agent(self, agent):
        logging.info("Stepping agent {}".format(agent.get_name()))
        agent.step()
        logging.info("End step agent {}".format(agent.get_name()))
