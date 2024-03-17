import utils
import numpy as np

# Main function.
def reward(world, agent, rewards):
    n_functions = len(rewards)
    if n_functions == 0:
        return 0
    else:
        total_reward = 0
        for r in rewards:
            f = r['function']
            args = r['args']
            total_reward += f(world, agent, **args) * r['weight']
        return total_reward

# Extrinsic reward function helpers.
def reward_sum(world, agent, variables, absolute=True, invert=False):
    sum = 0
    for v in variables:
        sum += reward_single(world, agent, v, absolute, invert)
    sum /= len(variables)
    return sum

def reward_single(world, agent, variable, absolute=True, invert=False):
    # Get standardized values (all in [0, 1]).
    data = world.get(agent, variable)
    # Option: for delta values, you can use absolute values centered at 0.5.
    if absolute:
        data = abs(data - 0.5) * 2 # Remap in [0, 1].
    # Reward equals data in [-1, 1].
    r = utils.map(data, 0, 1, -1, 1)
    # Option: invert.
    if invert:
        r = -r
    return r

def reward_delta_euler(world, agent):
    return reward_sum(world, agent, ['this.d_rx', 'this.d_ry', 'this.d_rz'], absolute=True, invert=False)

def reward_inv_delta_euler(world, agent):
    return reward_sum(world, agent, ['this.d_rx', 'this.d_ry', 'this.d_rz'], absolute=True, invert=True)

def reward_delta_position(world, agent):
    return reward_sum(world, agent, ['this.d_x', 'this.d_y'], absolute=True, invert=False)

def reward_inv_delta_position(world, agent):
    return reward_sum(world, agent, ['this.d_x', 'this.d_y'], absolute=True, invert=True)


def reward_get_close_thing1(world, agent):
    return reward_get_close(world, agent, 'thing1')

def reward_get_close_robot1(world, agent):
    return reward_get_close(world, agent, 'robot1')

def reward_get_close_robot2(world, agent):
    return reward_get_close(world, agent, 'robot2')

def reward_get_away_thing1(world, agent):
    return reward_get_away(world, agent, 'thing1')

def reward_get_away_robot1(world, agent):
    return reward_get_away(world, agent, 'robot1')

def reward_get_away_robot2(world, agent):
    return reward_get_away(world, agent, 'robot2')

# Reward difference between current and previous distance to target (with target radius, and specific values for stabilised positions).
def reward_get_close(world, agent, target):
    # NOTE: This should be parameterizable.
    dist = world.get(agent, 'dist_{}'.format(target), standardized=False)
    is_close = world.get(agent, 'close_{}'.format(target), standardized=False)
    delta_dist = world.get(agent, 'd_dist_{}'.format(target), standardized=False)
    print("*** DIST ***:", dist, delta_dist)

    # Define delta distance threshold (i.e., upon which the robot would be considered as stable, or not).
    delta_dist_threshold = 0.05 # 5cm

    # If robot close to target state:
    if is_close:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = -100

        # Give highest reward if robot stands still close to target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = 100

        # Give medium-high reward if robot gets closer to target
        elif delta_dist <= - delta_dist_threshold:
            reward = 10

    # If robot far away from target position:
    else:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = -100

        # Give middle low reward if robot stands still far away from target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = -100

        # Give medium-low reward if robot gets closer to target
        elif delta_dist <= -delta_dist_threshold:
            reward = -10

    return reward / 100.0


# Reward difference between current and previous distance to target (with target radius, and specific values for stabilised positions).
def reward_get_away(world, agent, target):
    # NOTE: This should be parameterizable.
    dist = world.get(agent, 'dist_{}'.format(target), standardized=False)
    is_close = world.get(agent, 'close_{}'.format(target), standardized=False)
    delta_dist = world.get(agent, 'd_dist_{}'.format(target), standardized=False)
    print("*** DIST ***:", dist, delta_dist)

    # Define delta distance threshold (i.e., upon which the robot would be considered as stable, or not).
    delta_dist_threshold = 0.05 # 5cm

    # If robot close to target state:
    if is_close:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = -10

        # Give highest reward if robot stands still close to target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = -10

        # Give medium-high reward if robot gets closer to target
        elif delta_dist <= - delta_dist_threshold:
            reward = -100

    # If robot far away from target position:
    else:

        # Give lowest reward if robot moves away from target
        if delta_dist > delta_dist_threshold:
            reward = 10

        # Give middle low reward if robot stands still far away from target
        elif -delta_dist_threshold < delta_dist <= delta_dist_threshold:
            reward = 100

        # Give medium-low reward if robot gets closer to target
        elif delta_dist <= -delta_dist_threshold:
            reward = -100

    return reward / 100.0

# Synchronize with other agents.
def reward_simple_sync(world, agent):
    # Get flash information.
    flash = world.get(agent, 'action', standardized=False)
    steps_since_flash = world.get(agent, 'steps_since_flash')
    steps_since_flash_absolute = world.get(agent, 'steps_since_flash', standardized=False)
    last_action = world.get(agent, 'last_action', standardized=False)
    action = world.get(agent, 'action', standardized=False)
    timer = world.get(agent, 'timer')

    flash_tolerance = 0.1
    flash_timeout = 0.5

    # Initialize reward.
    reward = 0

    # If you are flashing.
    if flash:
        if timer <= flash_tolerance:
            reward += 100
        # elif timer <= 2*flash_tolerance:
        #     reward += 10
        else:
            reward -= 10
    
    print("** flash *** agent={} f={} steps={}({}) last={} curr={} timer={} ==> reward={}".format(agent.get_name(), flash, steps_since_flash_absolute, steps_since_flash, last_action, action, timer, reward))

    return reward / 100.0


# Synchronize with other agents.
def reward_simple_sync2(world, agent):
    # Get flash information.
    flash = world.get(agent, 'flash')
    steps_since_flash = world.get(agent, 'steps_since_flash')
    steps_since_flash_absolute = world.get(agent, 'steps_since_flash', standardized=False)
    last_action = world.get(agent, 'last_action', standardized=False)
    action = world.get(agent, 'action', standardized=False)
    timer = world.get(agent, 'timer')

    flash_tolerance = 0.1
    flash_timeout = 0.5

    # Initialize reward.
    reward = 0

    # If you are flashing.
    if flash:
        if last_action == 1: # was already flashing
            reward -= 100
        else:
            if timer <= flash_tolerance:
                reward += 100
            elif timer <= 2*flash_tolerance:
                reward += 10
            else:
                reward -= 100
    
    print("** flash *** agent={} f={} steps={}({}) last={} curr={} timer={} ==> reward={}".format(agent.get_name(), flash, steps_since_flash_absolute, steps_since_flash, last_action, action, timer, reward))

    return reward / 100.0




