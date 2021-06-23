import utils

# Main function.
def reward(world, agent, reward_functions):
    n_functions = len(reward_functions)
    if n_functions == 0:
        return 0
    else:
        r = 0
        for f in reward_functions:
            r += f(world, agent)
        r /= n_functions
        return r

# Extrinsic reward function helpers.
def reward_sum(world, agent, variables, absolute=True, invert=False):
    # Get standardized values (all in [0, 1]).
    complete_data = world.get(agent, variables)
    # Option: for delta values, you can use absolute values centered at 0.5.
    if absolute:
        complete_data = abs(complete_data - 0.5) * 2
    # Compute averaged sum of values.
    r = sum(complete_data) / len(complete_data)
    # Option: invert.
    if invert:
        r = -r
    return r

def reward_delta_euler(world, agent):
    return reward_sum(world, agent, ['this.d_rx', 'this.d_ry', 'this.d_rz'], absolute=True, invert=False)

def reward_inv_delta_euler(world, agent):
    return reward_sum(world, agent, ['this.d_rx', 'this.d_ry', 'this.d_rz'], absolute=True, invert=True)