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

