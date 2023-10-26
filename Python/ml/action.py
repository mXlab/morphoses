import numpy as np
import utils
from chrono import Chrono

class ActionManager:
    
    def __init__(self, agent, world, action_profile, time_step=1, time_balance=0, navigation_mode=False, flash_mode=False):
        self.agent = agent
        self.world = world
        self.action_set = create_action_set(action_profile)
        self.time_step = time_step
        self.time_balance = time_balance

        self.navigation_mode = navigation_mode
        self.flash_mode = flash_mode

        self.chrono = Chrono(world)
        self.state = 0

        self.last_action_index = None

    def n_actions(self):
        if self.flash_mode:
            return 2
        else:
            return self.action_set.n_actions()

    def start_action(self, action_index):

        # Flash mode admits only two actions: flash (1) or no flash (0).
        if self.flash_mode:
            print("Flash mode =================")
            print("Last action = {}".format(self.last_action_index))
            # Flash: change action.
            if action_index == 1:
                print("Flashing!")
                # Pick a random action in set.
                real_action_index = self.action_set.random_action_index(exclude=self.last_action_index)
            
            # No flash: just keep doing previous action.
            else:
                print("Not flashing!")
                if self.last_action_index is None: # First action.
                    real_action_index = self.action_set.random_action_index()
                else:
                    real_action_index = self.last_action_index

            # Set action_index to new value for the rest of the processing.
            action_index = real_action_index
            print("New action = {}".format(action_index))
        
        # Get actual action from its index.
        action = self.action_set.get_action(action_index)

        speed = float(np.clip(action[0], -self.agent.get_max_speed(), self.agent.get_max_speed()))

        if self.navigation_mode:
            direction = utils.map(action[1], -1, +1, +90, -90)
            self.world.start_navigation(self.agent, speed, direction)
        else:
            steer = float(np.clip(action[1], -self.agent.get_max_steer(), self.agent.get_max_steer()))
            self.world.set_motors(self.agent, speed, steer)

        self.chrono.start()

        self.state = 0

        self.last_action_index = action_index

    def end_action(self):
        if self.navigation_mode:
            self.world.end_navigation(self.agent)
        else:
            self.world.set_speed(self.agent, 0)

    def step_action(self):
        # print("Step action {} {} {}".format(self.world.get_time(), self.start_time, self.time_step))
        # Check if first phase is finished.
        if self.state == 0:
            if self.chrono.has_passed(self.time_step):
                self.end_action()

                if self.time_balance > 0:
                    self.state = 1
                else:
                    self.state = 2
                # Restart chronometer.
                self.chrono.start()
            return True

        elif self.state == 1:
            if self.chrono.has_passed(self.time_balance):
                self.state = 2
            return True

        else:
            return False

class ActionSet:
    def __init__(self, actions=[]):
        self.actions = actions

    def add_action(self, speed, steer):
        self.actions.append( [ speed, steer ] )

    def n_actions(self):
        return len(self.actions)

    def get_action(self, i):
        return self.actions[i]

    def random_action_index(self, exclude=[]):
        if exclude is None:
            exclude = []
        elif isinstance(exclude, int):
            exclude = [exclude]
        i = np.random.randint(0, self.n_actions())
        while i in exclude:
            i = np.random.randint(0, self.n_actions())
        return i
    
    def random_action(self, exclude=[]):
        return self.get_action(self.random_action_index(exclude))
    

def create_action_set(action_profile):
    if action_profile == 'grid':
        return ActionSet(
                      [[+1, -1], [+1,  0], [+1, +1],
                       [ 0, -1], [ 0,  0], [ 0, +1],
                       [-1, -1], [-1,  0], [-1, +1]])
    elif action_profile == 'cross':
        return ActionSet(
                      [          [+1,  0],
                       [ 0, -1],           [ 0, +1],
                                 [-1, 0]])
    elif action_profile == 'cross+':
        return ActionSet(
                      [          [+1,  0],
                       [ 0, -1], [ 0,  0], [ 0, +1],
                                 [-1, 0]])
    elif action_profile == 'trident':
        return ActionSet(
                      [[+1, -0.2], [+1,  0], [+1, +0.1],
                                 [ 0,  0],
                                 [-1,  0]])
        # elif action_profile == 'trident':
        # return ActionSet(
        #     [[+1, -1], [+1, 0], [+1, +1],
        #      [0, 0],
        #      [-1, 0]])
    elif action_profile == 'forward':
        return ActionSet(
                      [[+1, -1], [+1,  0], [+1, +1],
                                 [ 0,  0]])
    elif action_profile == 'x':
        return ActionSet(
                      [[+1, -0.2],           [+1, +0.2],
                                 [ 0,  0],
                       [-1, -0.2],           [-1, +0.2]])

    elif action_profile == 'x-':
        return ActionSet(
                      [[+1, -0.5],           [+1, +0.5],

                       [-1, -0.5],           [-1, +0.5]])

    elif action_profile == 'h':
        return ActionSet(
                      [[+1, -0.5], [+1,  0], [+1, +0.5],
                                   [ 0,  0],
                       [-1, -0.5], [-1,  0], [-1, +0.5]])


    elif action_profile == 'i':
        return ActionSet(
                      [            [+1,  0],

                                    [-1,  0]            ] )

    elif action_profile == 'v':
        return ActionSet(
                      [          [+1,  0], [+1, +1],
                                 [ 0,  0]
                                                   ])

    elif action_profile == 'dance':
        return ActionSet(
                      [          [+1,  0],
                       [ 0, -1], [ 0,  -0.], [ 0, +1],
                                 [-1, 0]])

    elif action_profile == 'tilt':
        return ActionSet(
                      [         
                       [ 0, -1],             [ 0, +1]
                                        ])

    elif action_profile == 'line':
        return ActionSet(
                      [         
                       [ 0, -1],  [0, 0],    [ 0, +1]
                                        ])

    elif action_profile == 'still':
        return ActionSet([ [0, 0]] )

    else:
        raise RuntimeError("Unknown action profile: {}".format(action_profile))

