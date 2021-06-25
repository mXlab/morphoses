import numpy as np
import math
import time

from utils import *

import messaging

# Returns the difference between current and previous datapoints.
def delta(data, prev_data):
    two_pi = 2*math.pi
    d = data - prev_data
    d[6] = dist_angles(data[6]*two_pi, prev_data[6]*two_pi)
    d[7] = dist_angles(data[7]*math.pi, prev_data[7]*math.pi)
    d[8] = dist_angles(data[8]*two_pi, prev_data[8]*two_pi)
    return d

# Holds one data point, allowing to compute its delta and standardization.
class Data:
    def __init__(self,
                 min_value=-1, max_value=+1,
                 max_change_per_second=1,
                 auto_scale=True,
                 is_angle=False):
        self.value = self.prev_value = self.stored_value = 0
        self.delta_value = 0
        self.auto_scale = auto_scale
        if self.auto_scale:
            self.min_value = +9999
            self.max_value = -9999
            self.max_change_per_second = -9999
        else:
            self.min_value = min_value
            self.max_value = max_value
            self.max_change_per_second = max_change_per_second
        self.is_angle = is_angle
        self.stored_time = None
        self.prev_time = None

    # Temporarily store value at time t (in seconds).
    def store(self, value, t):
        self.stored_value = value
        self.stored_time = t
        if self.auto_scale:
            self.min_value = min(self.min_value, self.stored_value)
            self.max_value = max(self.max_value, self.stored_value)

    # Update values based on stored value.
    def update(self):
        if self.prev_time is None:
            interval = 0
        else:
            interval = self.stored_time - self.prev_time
        self.prev_time = self.stored_time
        self._update(self.stored_value, interval)

    # Get value.
    def get(self, standardized=True):
        if standardized:
            return map01(self.value, self.min_value, self.max_value)
        else:
            return self.value

    # Get delta value.
    def get_delta(self, standardized=True):
        if standardized:
            return map01(self.delta_value, -self.max_change_per_second, self.max_change_per_second)
        else:
            return self.delta_value

    # Internal use: updates everything.
    def _update(self, value, interval):
        self.prev_value = self.value
        self.value = value
        if interval > 0:
            if self.is_angle:
                delta_value = dist_angles(math.radians(value), math.radians(self.prev_value))
            else:
                delta_value = value - self.prev_value
            self.delta_value = delta_value / interval
        else:
            self.delta_value = 0
        if self.auto_scale:
            self.max_change_per_second = max(self.max_change_per_second, abs(self.delta_value))

    def __repr__(self):
        return str(self.stored_value)
#        return str( "s:{} v:{} d:{}".format(self.stored_value, self.value, self.delta_value))

class EntityData:
    def __init__(self):
        self.data = {}

    def add_data(self, label, **kwargs):
        self.data[label] = Data(**kwargs)

    def store(self, label, value, t):
        if isinstance(label, list):
            for i in range(len(label)):
                self.store(label[i], value[i], t)
        else:
            self.data[label].store(value, t)

    def update(self):
        # Update data.
        for d in self.data.values():
            d.update()

    def get(self, label):
        return self.data[label]

    def get_value(self, label, delta=False, standardized=True):
        data = self.get(label)
        if delta:
            return data.get_delta(standardized)
        else:
            return data.get(standardized)

    def store_polar(self, target_name, target, t):
        # Compute distance to target.
        distance = math.dist( (self.get_value('x', standardized=False), self.get_value('y', standardized=False)),
                              (target.get_value('x', standardized=False), target.get_value('y', standardized=False)) )
        self.store('dist_{}'.format(target_name), distance, t)

        # Computer vector for robot instantaneous direction.
        forward = np.sign(self.get_value('speed'))
        delta_pos = np.array([forward * self.get_value('x', delta=True), forward * self.get_value('y', delta=True)]) # invert according to current speed
        # print("velocity: ({}, {}) speed: {}".format(delta_pos[0], delta_pos[1], self.get_value('speed')))

        # Compute vector from robot to target state.
        delta_robot_to_target = np.array([target.get_value('x') - self.get_value('x'), target.get_value('y') - self.get_value('y')])

        # Normalize vectors.
        delta_pos = normalize(delta_pos)
        delta_robot_to_target = normalize(delta_robot_to_target)

        # Compute relative angle.
        dot_product = np.dot(delta_pos, delta_robot_to_target)
        angle = np.arccos(dot_product)
        angle = np.rad2deg(angle)
        # print("** target: {} drt: {} dp: {} angle: {}".format(target_name, delta_robot_to_target, delta_pos, angle))
        self.store('angle_{}'.format(target_name), angle, t)

    def __repr__(self):
        return str(self.data)

class RobotData(EntityData):
    def __init__(self, boundaries, entities, version):
        super().__init__()
        self.add_data('x', auto_scale=False, min_value=boundaries['x_min'], max_value=boundaries['x_max'])
        self.add_data('y', auto_scale=False, min_value=boundaries['y_min'], max_value=boundaries['y_max'])
        self.add_data('qx')
        self.add_data('qy')
        self.add_data('qz')
        self.add_data('qw')
        self.add_data('rx', is_angle=True, auto_scale=False, max_change_per_second=90)
        self.add_data('ry', is_angle=True, auto_scale=False, max_change_per_second=90)
        self.add_data('rz', is_angle=True, auto_scale=False, max_change_per_second=90)
        self.add_data('speed', min_value=-1, max_value=1, auto_scale=False)
        self.add_data('steer', min_value=-1, max_value=1, auto_scale=False)

        max_dist = math.dist( (boundaries['x_min'], boundaries['y_min']), (boundaries['x_max'], boundaries['y_max']) )
        max_dist *= 0.5 # let's be realistic
        for name in entities:
            self.add_data('dist_{}'.format(name), auto_scale=False, min_value=0, max_value=max_dist)
            self.add_data('angle_{}'.format(name), is_angle=True, auto_scale=False, min_value=0, max_value=360)
        self.version = version

    def get_version(self):
        return self.version

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

    def store_quaternion(self, quat, t):
        self.store(['qx', 'qy', 'qz', 'qw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['rx', 'ry', 'rz'], [rx, ry, rz], t)

    def store_quaternion_main(self, quat, t):
        self.store(['mqx', 'mqy', 'mqz', 'mqw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['mrx', 'mry', 'mrz'], [rx, ry, rz], t)

    def store_action(self, action, t):
        self.store(['speed', 'steer'], action, t)

class ThingData(EntityData):
    def __init__(self, boundaries):
        super().__init__()
        self.add_data('x', auto_scale=False, min_value=boundaries['x_min'], max_value=boundaries['x_max'])
        self.add_data('y', auto_scale=False, min_value=boundaries['y_min'], max_value=boundaries['y_max'])

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

class World:
    def __init__(self, settings):
        self.start_time = time.time()
        self.entities = {}
        for robot in settings['robots']:
            robot_name = robot['name']
            self.entities[robot_name] = RobotData(robot['version'], settings['boundaries'])
        for thing in settings['things']:
            thing_name = thing['name']
            self.entities[thing_name] = ThingData()

        self.messaging = messaging.Messaging(self, settings)

        self.max_speed = settings['motors']['max_speed']
        self.max_steer = settings['motors']['max_steer']
        self.virtual_boundaries = settings['virtual_boundaries']

    def get(self, agent, variable, standardized=True):
        # Process variables as list.
        if isinstance(variable, list):
            values = []
            for v in variable:
                values.append(self.get(agent, v, standardized))
            return np.array(values)

        # Process single variable.
        else:
            entity_name, variable, delta = self._get_variable_info(agent, variable)
            data = self.entities[entity_name].get(variable)
            if delta:
                return data.get_delta(standardized)
            else:
                return data.get(standardized)

    def _get_variable_info(self, agent, variable):
        # Extract entity from 'entity.variable' format.
        if '.' in variable:
            entity_name, var = variable.split('.')
            if entity_name == 'this':
                entity_name = agent.get_name()
            variable = var
        else:
            entity_name = agent.get_name()
        # Check delta variables.
        if variable.startswith('d_'):
            variable = variable[2:]  # remove the 'd_' part
            delta = True
        else:
            delta = False
        # Return info as tuple.
        return entity_name, variable, delta

    def do_action(self, agent, action):
        # Store action in entity.
        self.entities[agent.get_name()].store_action(action, self.get_time())
        # Perform actual action.
        speed = np.clip(action[0], -agent.get_max_speed(), agent.get_max_speed())
        steer = np.clip(action[1], -agent.get_max_steer(), agent.get_max_steer())
        self.set_motors(agent, speed, steer)

    def set_motors(self, agent, speed, steer):
        if isinstance(agent, str):
            name = agent
        else:
            name = agent.get_name()
        if self.entities[name].get_version() >= 3:
            self.messaging.send(name, "/speed", speed)
            self.messaging.send(name, "/steer", steer)
        else:
            self.messaging.send(name, "/motor/1", round(speed*128))
            self.messaging.send(name, "/motor/2", round(steer*90))

    def set_color(self, agent, rgb):
        if isinstance(agent, str):
            name = agent
        else:
            name = agent.get_name()
        if self.entities[name].get_version() >= 3:
            self.messaging.send(name, "/rgb", rgb)
        else:
            self.messaging.send(name, "/red",   rgb[0])
            self.messaging.send(name, "/green", rgb[1])
            self.messaging.send(name, "/blue",  rgb[2])

    def is_inside_boundaries(self, agent):
        x = self.get(agent, 'x', standardized=False)
        y = self.get(agent, 'y', standardized=False)
        return self.virtual_boundaries['x_min'] <= x and x <= self.virtual_boundaries['x_max'] and self.virtual_boundaries['y_min'] <= y and y <= self.virtual_boundaries['y_max']

    def step(self):
        self.messaging.loop()
        self.update()
        self.debug()

    def sleep(self, t):
        start_time = time.time()
        while time.time() - start_time < t:
            self.messaging.loop()

    def terminate(self):
        self.messaging.terminate()

    def update(self):
        for entity in self.entities.values():
            entity.update()

    def get_time(self):
        return time.time() - self.start_time

    def store_position(self, entity_name, pos):
        t = self.get_time()
        entity = self.entities[entity_name]
        entity.store_position(pos, t)

        # Update distances relative to other robots and entities.
        for name in self.robots:
            other_robot = self.entities[name]
            other_robot.store_polar(entity_name, entity, t)
            entity.store_polar(name, other_robot, t)

    def store_quaternion(self, entity_name, quat):
        self.entities[entity_name].store_quaternion(quat, self.get_time())

    def store_quaternion_main(self, entity_name, quat):
        self.entities[entity_name].store_quaternion_main(quat, self.get_time())

    def debug(self):
        print(self.entities)