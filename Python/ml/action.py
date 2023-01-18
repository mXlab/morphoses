import numpy as np
import utils

class ActionManager:
    def __init__(self, agent, world, action_profile, time_step=1, time_balance=0, navigation_mode=False):
        self.agent = agent
        self.world = world
        self.action_set = create_action_set(action_profile)
        self.time_step = time_step
        self.time_balance = time_balance
        self.navigation_mode = navigation_mode
        self.start_time = 0
        self.state = 0

    def n_actions(self):
        return self.action_set.n_actions()

    def start_action(self, i):
        action = self.action_set.get_action(i)

        speed = float(np.clip(action[0], -self.agent.get_max_speed(), self.agent.get_max_speed()))

        if self.navigation_mode:
            direction = utils.map(action[1], -1, +1, +90, -90)
            self.world.start_navigation(self.agent, speed, direction)
        else:
            steer = float(np.clip(action[1], -self.agent.get_max_steer(), self.agent.get_max_steer()))
            self.world.set_motors(self.agent, speed, steer)

        self.start_time = self.world.get_time()
        self.state = 0

    def end_action(self):
        if self.navigation_mode:
            self.world.end_navigation(self.agent)
        else:
            self.world.set_speed(self.agent, 0)

    def step_action(self):
        # print("Step action {} {} {}".format(self.world.get_time(), self.start_time, self.time_step))
        # Check if first phase is finished.
        if self.state == 0:
            if self.world.get_time() - self.start_time >= self.time_step:
                self.end_action()

                if self.time_balance > 0:
                    self.state = 1
                else:
                    self.state = 2
                self.start_time = self.world.get_time()
            return True

        elif self.state == 1:
            if self.world.get_time() - self.start_time >= self.time_balance:
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
                      [[+1, -1], [+1,  0], [+1, +1],
                                 [ 0,  0],
                                 [-1,  0]])
    elif action_profile == 'forward':
        return ActionSet(
                      [[+1, -1], [+1,  0], [+1, +1],
                                 [ 0,  0]])
    elif action_profile == 'x':
        return ActionSet(
                      [[+1, -0.5],           [+1, +0.5],
                                 [ 0,  0],
                       [-1, -0.5],           [-1, +0.5]])

    elif action_profile == 'h':
        return ActionSet(
                      [[+1, -1], [+1,  0], [+1, +1],
                                 [ 0,  0],
                       [-1, -1], [-1,  0], [-1, +1]])

    elif action_profile == 'still':
        return ActionSet([ [0, 0]] )

    else:
        raise RuntimeError("Unknown action profile: {}".format(action_profile))

