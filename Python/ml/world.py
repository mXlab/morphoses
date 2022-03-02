import numpy as np
import math
import time

from utils import *

import messaging

# Holds one data point, allowing to compute its delta and standardization.
class Data:
    def __init__(self,
                 min_value=-1, max_value=+1,
                 max_change_per_second=None,
                 auto_scale=True,
                 auto_scale_delta=True,
                 is_angle=False,
                 smoothing=0.0):
        self.value = self.prev_value = self.stored_value = None
        self.delta_value = 0
        self.auto_scale = auto_scale
        self.auto_scale_delta = auto_scale_delta
        if self.auto_scale:
            self.min_value = +9999
            self.max_value = -9999
        else:
            self.min_value = min_value
            self.max_value = max_value
        if max_change_per_second is None:
            if self.auto_scale_delta:
                self.max_change_per_second = -9999
            else:
                self.max_change_per_second = 1
        else:
            self.max_change_per_second = max_change_per_second
        self.is_angle = is_angle
        self.stored_time = None
        self.prev_time = None
        self.smoothing = np.clip(smoothing, 0.0, 1.0)

    def is_valid(self):
        return self.value is not None

    # Temporarily store value at time t (in seconds).
    def store(self, value, t):
        if self.stored_time is None:
            self.stored_value = value
        else:
            # Smooth using an EMA. Adjusts smoothing factor based on: smoothing = base_smoothing ^ delta_time
            smoothing = 1.0 - np.clip(math.pow(self.smoothing, t - self.stored_time), 0.0, 1.0)
            self.stored_value += smoothing * (value - self.stored_value)
            # self.stored_value = value
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
        if self.auto_scale_delta:
            self.max_change_per_second = max(self.max_change_per_second, abs(self.delta_value))

    def __repr__(self):
        return str(self.stored_value)
#        return str( "s:{} v:{} d:{}".format(self.stored_value, self.value, self.delta_value))

class EntityData:
    def __init__(self):
        self.data = {}

    def add_data(self, label, **kwargs):
        self.data[label] = Data(**kwargs)

    def is_valid(self, labels):
        if not isinstance(labels, list):
            labels = [ labels ]
        for var in labels:
            if not self.data[var].is_valid():
                return False
        return True

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

    def store_polar(self, target_name, target, close_dist, t):
        # Compute distance to target.
        if self.is_valid(['x', 'y']) and target.is_valid(['x', 'y']):
            x = self.get_value('x', standardized=False)
            y = self.get_value('y', standardized=False)
            tx = target.get_value('x', standardized=False)
            ty = target.get_value('y', standardized=False)
            distance = math.dist( (self.get_value('x', standardized=False), self.get_value('y', standardized=False)),
                                  (target.get_value('x', standardized=False), target.get_value('y', standardized=False)) )
            self.store('dist_{}'.format(target_name), distance, t)

            # Compute the "is close" state.
            is_close = 1.0 if distance < close_dist else 0
            self.store('close_{}'.format(target_name), is_close, t)

            # Compute vector from robot to target state.
            angle_robot_to_target = np.rad2deg(math.atan2(target.get_value('y') - self.get_value('y'), target.get_value('x') - self.get_value('x')))
            if self.is_valid('mrz'):
                heading = self.get_value('mrz', standardized=False)
                angle = wrap_angle_180(angle_robot_to_target - heading)
                #print("** target: {} drt: {} dp: {} angle: {}".format(target_name, delta_robot_to_target, delta_pos, angle))
                self.store('angle_{}'.format(target_name), angle, t)
                self.store("quadrant_{}".format(target_name), quadrant(angle), t)

    def __repr__(self):
        return str(self.data)

class RobotData(EntityData):
    def __init__(self, boundaries, entities, version):
        super().__init__()
        self.add_data('x', auto_scale=False, max_change_per_second=0.01, min_value=boundaries['x_min'], max_value=boundaries['x_max'], smoothing=0.1)
        self.add_data('y', auto_scale=False, max_change_per_second=0.01, min_value=boundaries['y_min'], max_value=boundaries['y_max'], smoothing=0.1)
        self.add_data('qx')
        self.add_data('qy')
        self.add_data('qz')
        self.add_data('qw')
        self.add_data('rx', is_angle=True)
        self.add_data('ry', is_angle=True)
        self.add_data('rz', is_angle=True)

        self.add_data('mqx')
        self.add_data('mqy')
        self.add_data('mqz')
        self.add_data('mqw')
        self.add_data('mrx', is_angle=True)
        self.add_data('mry', is_angle=True)
        self.add_data('mrz', is_angle=True)

        self.add_data('speed', min_value=-1, max_value=1, max_change_per_second=2, auto_scale=False, auto_scale_delta=False)
        self.add_data('steer', min_value=-1, max_value=1, max_change_per_second=2, auto_scale=False, auto_scale_delta=False)
        # self.add_data('rx', is_angle=True, auto_scale=False, max_change_per_second=90)
        # self.add_data('ry', is_angle=True, auto_scale=False, max_change_per_second=90)
        # self.add_data('rz', is_angle=True, auto_scale=False, max_change_per_second=90)

        max_dist = math.dist( (boundaries['x_min'], boundaries['y_min']), (boundaries['x_max'], boundaries['y_max']) )
        max_dist *= 0.5 # let's be realistic
        for name in entities:
            self.add_data('dist_{}'.format(name), auto_scale=False, min_value=0, max_value=max_dist)
            self.add_data('close_{}'.format(name), auto_scale=False, auto_scale_delta=False, min_value=0, max_value=1, max_change_per_second=1)
            self.add_data('angle_{}'.format(name), is_angle=True, auto_scale=False, min_value=-180, max_value=180)
            self.add_data('quadrant_{}'.format(name), auto_scale=False, min_value=0, max_value=3)
        self.version = version

    def get_version(self):
        return self.version

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

    def store_quaternion(self, quat, t):
        self.store(['qx', 'qy', 'qz', 'qw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['rx', 'ry', 'rz'], [rx, ry, rz], t)

    def store_quaternion_main(self, quat, t, zOffset):
        self.store(['mqx', 'mqy', 'mqz', 'mqw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3], zOffset)
        self.store(['mrx', 'mry', 'mrz'], [rx, ry, rz], t)

    def store_action(self, action, t):
        self.store(['speed', 'steer'], action, t)

class ThingData(EntityData):
    def __init__(self, boundaries):
        super().__init__()
        self.add_data('x', auto_scale=False, min_value=boundaries['x_min'], max_value=boundaries['x_max'], smoothing=0.1)
        self.add_data('y', auto_scale=False, min_value=boundaries['y_min'], max_value=boundaries['y_max'], smoothing=0.1)

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

class World:
    def __init__(self, settings):
        # Build lists of entities.
        self.robots = []
        self.things = []
        for robot in settings['robots']:
            robot_name = robot['name']
            self.robots.append(robot_name)
        for thing in settings['things']:
            thing_name = thing['name']
            self.things.append(thing_name)
        entities_list = self.robots + self.things + ['center']

        # Create entities data structures (robots & things).
        self.entities = {}
        for robot in settings['robots']:
            robot_name = robot['name']
            self.entities[robot_name] = RobotData(settings['boundaries'], entities_list, robot['version'])
        for thing in settings['things']:
            thing_name = thing['name']
            self.entities[thing_name] = ThingData(settings['boundaries'])

        self.entities['center'] = ThingData(settings['boundaries'])
        vb = settings['virtual_boundaries']
        x_center = 0.5 * (vb['x_min'] + vb['x_max'])
        y_center = 0.5 * (vb['y_min'] + vb['y_max'])
        self.entities['center'].store_position([ x_center, y_center ], 0)

        # Create messaging system.
        self.messaging = messaging.Messaging(self, settings)

        # # Set max speed and steer.
        # self.max_speed = settings['motors']['max_speed']
        # self.max_steer = settings['motors']['max_steer']

        self.virtual_boundaries = settings['virtual_boundaries']
        self.close_dist = settings['close_dist']

        self.room_heading = settings['room_heading']

        # Save current starting time.
        self.start_time = time.time()

    def is_valid(self, agent, variable):
        # Process variables as list.
        if isinstance(variable, list):
            values = []
            valid = True
            for v in variable:
                if not self.is_valid(agent, v):
                    print("Invalid variable for agent {}: {}".format(agent, v))
                    valid = False
            return valid

        # Process single variable.
        else:
            entity_name, variable, __ = self._get_variable_info(agent, variable)
            data = self.entities[entity_name].get(variable)
            return data.is_valid()

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
        speed = float(np.clip(action[0], -agent.get_max_speed(), agent.get_max_speed()))
        steer = float(np.clip(action[1], -agent.get_max_steer(), agent.get_max_steer()))
        self.set_motors(agent, speed, steer)

        # self.send_info('robot1', '/data', [self.get(agent, 'x'), self.get(agent, 'y'), self.get(agent, 'mrz', False)])

    def set_speed(self, agent, speed):
        if isinstance(agent, str):
            name = agent
        else:
            name = agent.get_name()
        entity = self.entities[name]
        if entity.get_version() >= 3:
            self.messaging.send(name, "/speed", speed)
        else:
            self.messaging.send(name, "/motor/1", round(speed*128))

    def set_steer(self, agent, steer):
        if isinstance(agent, str):
            name = agent
        else:
            name = agent.get_name()
        entity = self.entities[name]
        if entity.get_version() >= 3:
            self.messaging.send(name, "/steer", steer)
        else:
            self.messaging.send(name, "/motor/2", round(steer*90))

    def set_motors(self, agent, speed, steer):
        self.set_speed(agent, speed)
        self.set_steer(agent, steer)

    def set_color(self, agent, rgb):
        if isinstance(agent, str):
            name = agent
        else:
            name = agent.get_name()
        entity = self.entities[name]
        if entity.get_version() >= 3:
            self.messaging.send(name, "/rgb-region", [2] + rgb)
            # self.messaging.send(name, "/rgb", rgb)
        else:
            self.messaging.send(name, "/red",   rgb[0])
            self.messaging.send(name, "/green", rgb[1])
            self.messaging.send(name, "/blue",  rgb[2])

    def is_inside_boundaries(self, agent, use_recenter_offset=True):
        x = self.get(agent, 'x', standardized=False)
        y = self.get(agent, 'y', standardized=False)
        offset = self.virtual_boundaries['recenter_offset'] if use_recenter_offset else 0
        return self.virtual_boundaries['x_min']+offset <= x and \
               x <= self.virtual_boundaries['x_max']-offset and \
               self.virtual_boundaries['y_min']+offset <= y and \
               y <= self.virtual_boundaries['y_max']-offset

    def begin(self):
        print("Messaging begin")
        self.messaging.begin()

        print("Init robots")
        for robot in self.robots:
            self.set_motors(robot, 0, 0)
            self.entities[robot].store_action([0, 0], self.get_time())
            self.set_color(robot, [0, 255, 255])
            self.messaging.send(robot, "/power", 1)
            self.messaging.send(robot, "/stream", 1)
            self.messaging.send(robot, "/stream", 1, board_name='imu')

        # Call an update to initialize data.
        print("Init data")
        self.update()

    def step(self):
        self.messaging.loop()
        self.update()
        self.debug()

    def sleep(self, t):
        start_time = time.time()
        while time.time() - start_time < t:
            self.messaging.loop()

    def terminate(self):
        for robot in self.robots:
            self.set_motors(robot, 0, 0)
            self.set_color(robot, [0, 0, 0])
            # self.messaging.send(robot, "/power", 0)
            # self.messaging.send(robot, "/stream", 0)
            # self.messaging.send(robot, "/stream", 0, board_name='imu')
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

        # Update distances relative to other robots.
        for name in self.robots:
            other_robot = self.entities[name]
            other_robot.store_polar(entity_name, entity, self.close_dist, t)

        if entity_name in self.robots:
            for name in self.entities:
                entity.store_polar(name, self.entities[name], self.close_dist, t)

    def store_quaternion(self, entity_name, quat):
        self.entities[entity_name].store_quaternion(quat, self.get_time())

    def store_quaternion_main(self, entity_name, quat):
        # Correct euler yaw with room heading offset.
        self.entities[entity_name].store_quaternion_main(quat, self.get_time(), -self.room_heading)

    def send_info(self, entity_name, address, args):
        self.messaging.send_info("{}/{}".format(address, entity_name), args)

    def debug(self):
        print(self.entities)