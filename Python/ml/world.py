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
                 smoothing=0.0,
                 default=None):
        self.value = self.prev_value = self.stored_value = default
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
            return inv_lerp(self.value, self.min_value, self.max_value)
        else:
            return self.value

    # Get delta value.
    def get_delta(self, standardized=True):
        if standardized:
            return inv_lerp(self.delta_value, -self.max_change_per_second, self.max_change_per_second)
        else:
            return self.delta_value

    # Internal use: updates everything.
    def _update(self, value, interval):
        # Swap prev and current values.
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
        if self.stored_value != None:
            return str(self.stored_value)
        else:
            return "null"


class EntityData:
    def __init__(self):
        self.data = {}
        self.groups = {}

    def add_data(self, label, **kwargs):
        self.data[label] = Data(**kwargs)

    def is_valid(self, labels=None):
        if labels is None:  # default = everything should be valid
            labels = list(self.data.keys())
        if not isinstance(labels, list):
            labels = [labels]
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

    def get_values(self, labels, delta=False, standardized=True):
        return [self.get_value(l, delta, standardized) for l in labels]

    def add_group(self, group, labels):
        self.groups[group] = labels

    def get_group_values(self, group, delta=False, standardized=True):
        return self.get_values(self.groups[group], delta, standardized)

    def group_is_valid(self, group):
        return self.is_valid(self.groups[group])

    def store_polar(self, target_name, target, close_dist, t):
        # Compute distance to target.
        if self.group_is_valid('position') and target.group_is_valid('position'):
            x = self.get_value('x', standardized=False)
            y = self.get_value('y', standardized=False)
            tx = target.get_value('x', standardized=False)
            ty = target.get_value('y', standardized=False)
            distance = math.dist((self.get_value('x', standardized=False), self.get_value('y', standardized=False)),
                                 (target.get_value('x', standardized=False), target.get_value('y', standardized=False)))
            self.store('dist_{}'.format(target_name), distance, t)

            # Compute the "is close" state.
            is_close = 1.0 if distance < close_dist else 0
            self.store('close_{}'.format(target_name), is_close, t)

            # Compute vector from robot to target state.
            if self.is_valid('mrz'):
                heading = self.get_value('mrz', standardized=False)
                angle = target_heading(self.get_value('x', standardized=False), self.get_value('y', standardized=False),
                                       heading,
                                       target.get_value('x', standardized=False),
                                       target.get_value('y', standardized=False))
                # print("** target: {} drt: {} dp: {} angle: {}".format(target_name, delta_robot_to_target, delta_pos, angle))
                self.store('angle_{}'.format(target_name), angle, t)
                self.store("quadrant_{}".format(target_name), quadrant(angle), t)

    def __repr__(self):
        return str(self.data)


class RobotData(EntityData):
    def __init__(self, boundaries, entities, version):
        super().__init__()
        self.add_group('position', ['x', 'y'])
        self.add_data('x', auto_scale=False, max_change_per_second=0.01, min_value=boundaries['x_min'],
                      max_value=boundaries['x_max'], smoothing=0.1)
        self.add_data('y', auto_scale=False, max_change_per_second=0.01, min_value=boundaries['y_min'],
                      max_value=boundaries['y_max'], smoothing=0.1)

        self.add_group('quaternion_side', ['qx', 'qy', 'qz', 'qw'])
        self.add_data('qx')
        self.add_data('qy')
        self.add_data('qz')
        self.add_data('qw')

        self.add_group('rotation_side', ['rx', 'ry', 'rz'])
        self.add_data('rx', is_angle=True)
        self.add_data('ry', is_angle=True)
        self.add_data('rz', is_angle=True)

        self.add_group('quaternion_main', ['mqx', 'mqy', 'mqz', 'mqw'])
        self.add_data('mqx')
        self.add_data('mqy')
        self.add_data('mqz')
        self.add_data('mqw')

        self.add_group('rotation_main', ['mrx', 'mry', 'mrz'])
        self.add_data('mrx', is_angle=True)
        self.add_data('mry', is_angle=True)
        self.add_data('mrz', is_angle=True)

        self.add_group('motors', ['speed', 'steer'])
        self.add_data('speed', min_value=-1, max_value=1, max_change_per_second=2, auto_scale=False,
                      auto_scale_delta=False)
        self.add_data('steer', min_value=-1, max_value=1, max_change_per_second=2, auto_scale=False,
                      auto_scale_delta=False)
        
        self.add_group('actions', ['last_action', 'action', 'flash', 'steps_since_flash'])
        self.add_data('last_action', default=0)
        self.add_data('action')
        self.add_data('flash', min_value=0, max_value=1, auto_scale=False, default=0)
        self.add_data('steps_since_flash', min_value=0, max_value=20, auto_scale=False, default=0)
        self.add_data('timer', min_value=0, max_value=1, auto_scale=False, default=0)

        # self.add_data('last_action')
        # self.add_data('rx', is_angle=True, auto_scale=False, max_change_per_second=90)
        # self.add_data('ry', is_angle=True, auto_scale=False, max_change_per_second=90)
        # self.add_data('rz', is_angle=True, auto_scale=False, max_change_per_second=90)

        max_dist = math.dist((boundaries['x_min'], boundaries['y_min']), (boundaries['x_max'], boundaries['y_max']))
        max_dist *= 0.5  # let's be realistic: it will be rare that target is completely at the other side of the room

        for name in entities:
            self.add_data('dist_{}'.format(name), auto_scale=False, min_value=0, max_value=max_dist)
            self.add_data('close_{}'.format(name), auto_scale=False, auto_scale_delta=False, min_value=0, max_value=1,
                          max_change_per_second=1)
            self.add_data('angle_{}'.format(name), is_angle=True, auto_scale=False, min_value=-180, max_value=180)
            self.add_data('quadrant_{}'.format(name), auto_scale=False, min_value=0, max_value=3)
        self.version = version

    def get_version(self):
        return self.version

    def get_position(self, delta=False, standardized=True):
        return self.get_values(['x', 'y'], delta, standardized)

    def get_quaternion_side(self, delta=False, standardized=True):
        return self.get_values(['qx', 'qy', 'qz', 'qw'], delta, standardized)

    def get_quaternion_main(self, delta=False, standardized=True):
        return self.get_values(['mqx', 'mqy', 'mqz', 'mqw'], delta, standardized)

    def get_rotation_side(self, delta=False, standardized=True):
        return self.get_values(['rx', 'ry', 'rz'], delta, standardized)

    def get_rotation_main(self, delta=False, standardized=True):
        return self.get_values(['mrx', 'mry', 'mrz'], delta, standardized)

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

    def store_quaternion_side(self, quat, t):
        self.store(['qx', 'qy', 'qz', 'qw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['rx', 'ry', 'rz'], [rx, ry, rz], t)

    def store_quaternion_main(self, quat, t, zOffset):
        self.store(['mqx', 'mqy', 'mqz', 'mqw'], quat, t)
        rx, ry, rz = quaternion_to_euler(quat[0], quat[1], quat[2], quat[3], zOffset)
        self.store(['mrx', 'mry', 'mrz'], [rx, ry, rz], t)

    def store_action(self, action, t):
        # If this is the first action, just store it.
        if not self.get('last_action').is_valid():
            flash = 0
            steps_since_flash = 0
            last_action = action

        # Else, compare with previous action.
        else:
            if self.get('action').is_valid():
                last_action = int(self.get('action').get(False))
            else:
                last_action = 0
            flash = (action != last_action)
            if flash:
                steps_since_flash = 0
            else:
                steps_since_flash = self.get('steps_since_flash').get(False) + 1
        
        timer_max_time = 10.0
        timer = (t % timer_max_time) / timer_max_time

        # Store action.
        self.store('last_action', last_action, t)
        self.store('action', action, t)
        self.store('flash', flash, t)
        self.store('steps_since_flash', steps_since_flash, t)
        self.store('timer', timer, t)

        # # Get current speed and steer.
        # current_speed = self.get('speed').get(False)
        # current_steer = self.get('steer').get(False)

        # # Compare action with current speed and steer.
        # speed = action[0]
        # steer = action[1]
        # flash = (speed != current_speed and steer != current_steer)

        # # Store action.
        # self.store(['speed', 'steer'], action, t)


class ThingData(EntityData):
    def __init__(self, boundaries):
        super().__init__()
        self.add_group('position', ['x', 'y'])
        self.add_data('x', auto_scale=False, min_value=boundaries['x_min'], max_value=boundaries['x_max'],
                      smoothing=0.1)
        self.add_data('y', auto_scale=False, min_value=boundaries['y_min'], max_value=boundaries['y_max'],
                      smoothing=0.1)

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
        self.entities['center'].store_position([x_center, y_center], 0)

        self.use_virtual_boundaries = vb['use']

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

    def set_manager(self, manager):
        self.messaging.set_manager(manager)

    def is_valid(self, agent, variable):
        # Process variables as list.
        if isinstance(variable, list):
            valid = True
            for v in variable:
                if not self.is_valid(agent, v):
                    print("Invalid variable for agent {}: {}".format(agent.get_name(), v))
                    valid = False
            return valid

        # Process single variable.
        else:
            entity_name, variable, __ = self._get_variable_info(agent, variable)
            data = self.entities[entity_name].get(variable)
            return data.is_valid()

    def get_robots(self):
        return self.robots

    def get_things(self):
        return self.things

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
            entity_name = self.agent_as_name(agent)

        # Check delta variables.
        if variable.startswith('d_'):
            variable = variable[2:]  # remove the 'd_' part
            delta = True
        else:
            delta = False
        # Return info as tuple.
        return entity_name, variable, delta

    def agent_as_name(self, agent):
        return agent if isinstance(agent, str) else agent.get_name()

    def do_action(self, agent, action, action_manager):
        # Store action in entity.
        # self.entities[agent.get_name()].store_action(action, self.get_time())
        action_manager.start_action(action)
        while action_manager.step_action():
            self.messaging.loop()

    def set_speed(self, agent, speed):
        name = self.agent_as_name(agent)
        entity = self.entities[name]
        if entity.get_version() >= 3:
            print("set speed: {}".format(speed))
            self.messaging.send(name, "/speed", speed)
        else:
            self.messaging.send(name, "/motor/1", round(speed * 128))

    def set_steer(self, agent, steer):
        name = self.agent_as_name(agent)
        entity = self.entities[name]
        if entity.get_version() >= 3:
            self.messaging.send(name, "/steer", steer)
        else:
            self.messaging.send(name, "/motor/2", round(steer * 90))

    def set_motors(self, agent, speed, steer):
        self.set_speed(agent, speed)
        self.set_steer(agent, steer)

    def start_navigation(self, agent, speed, direction):
        name = self.agent_as_name(agent)
        entity = self.entities[name]
        self.messaging.send(name, "/nav/start", [speed, map(direction, -1, 1, 90, -90)])

    def end_navigation(self, agent):
        name = self.agent_as_name(agent)
        entity = self.entities[name]
        self.messaging.send(name, "/nav/stop")

    # Stop mode: depending on success.
    def display_stop(self, agent, success):
        if success:
            alt_color = [4, 16, 0]
        else:
            alt_color = [20, 4, 0]
        # Calculate color representative of reward.
        animation = {
            "base": [32, 32, 16],
            "alt": alt_color,
            "period": 4,
            "noise": 0.1,
            "region": 0,
            "type": 0
        }

        # Send animation parameters.
        name = self.agent_as_name(agent)
        self.messaging.send_animation(name, animation)

    # Idle mode (between behaviors).
    def display_idle(self, agent):
        animation = {
            "base": [8, 4, 0],
            "alt": [0, 0, 0],
            "period": 8,
            "noise": 0.1,
            "region": 0,
            "type": 0
        }

        # Send animation parameters.
        name = self.agent_as_name(agent)
        self.messaging.send_animation(name, animation)

    # Recenter mode.
    def display_recenter(self, agent):
        # Calculate color representative of reward.
        animation = {
            "base": [64, 48, 32],
            "alt": [48, 32, 16],
            "period": 2,
            "noise": 0.4,
            "region": 2,
            "type": 1
        }

        # Send animation parameters.
        name = self.agent_as_name(agent)
        self.messaging.send_animation(name, animation)

    # Reward.
    def display_reward(self, agent, scaled_reward):
        # Calculate color representative of reward.
        animation = {
            'base': lerp_color(scaled_reward, [255, 8, 0], [96, 255, 48]),
            'alt': lerp_color(scaled_reward, [16, 0, 0], [4, 32, 0]),
            'period': lerp(scaled_reward, 0.5, 3),
            'noise': lerp(scaled_reward, 0.4, 0.2),
            'region': 2 if scaled_reward < 0.5 else 0,
            'type': 1
        }

        # Send animation parameters.
        name = self.agent_as_name(agent)
        self.messaging.send_animation(name, animation)

    def display(self, agent, state, reward, scaled_reward):
        # Display scaled reward.
        self.display_reward(agent, scaled_reward)

        # Broadcast as OSC.
        name = self.agent_as_name(agent)
        info = state[0].tolist() + [reward]
        self.send_info(name, "/info", info)

    def set_animation_period(self, agent, period):
        name = self.agent_as_name(agent)
        self.messaging.send(name, "/animation/period", period)

    def set_animation_noise(self, agent, noise):
        name = self.agent_as_name(agent)
        self.messaging.send(name, "/animation/noise", [noise, 0])

    def set_alt_color(self, agent, rgb):
        name = self.agent_as_name(agent)
        self.messaging.send(name, "/animation/to", rgb)

    def set_color(self, agent, rgb):
        name = self.agent_as_name(agent)
        entity = self.entities[name]
        if entity.get_version() >= 3:
            self.messaging.send(name, "/animation/region", 2)
            self.messaging.send(name, "/animation/from", rgb)
            # self.messaging.send(name, "/rgb-region", [2] + rgb)
            # self.messaging.send(name, "/rgb", rgb)
        else:
            self.messaging.send(name, "/red", rgb[0])
            self.messaging.send(name, "/green", rgb[1])
            self.messaging.send(name, "/blue", rgb[2])

    def is_inside_boundaries(self, agent, use_recenter_offset=True):
        if (not self.is_valid(agent, 'x') or not self.is_valid(agent, 'y')):
            return True
        if not self.use_virtual_boundaries:
            return True
        x = self.get(agent, 'x', standardized=False)
        y = self.get(agent, 'y', standardized=False)
        offset = self.virtual_boundaries['recenter_offset'] if use_recenter_offset else 0
        return (self.virtual_boundaries['x_min'] + offset <= x <= self.virtual_boundaries['x_max'] - offset) and \
                (self.virtual_boundaries['y_min'] + offset <= y <= self.virtual_boundaries['y_max'] - offset)

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
        self.send_data()

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

        for robot_name in self.robots:
            robot = self.entities[robot_name]
            if robot.group_is_valid('position'):
                self.send_info(robot_name, "/pos", robot.get_position())
            if robot.group_is_valid('quaternion_side'):
                self.send_info(robot_name, "/side/quat", robot.get_quaternion_side(standardized=False))
            if robot.group_is_valid('rotation_side'):
                self.send_info(robot_name, "/side/rot", robot.get_rotation_side(standardized=False))
            if robot.group_is_valid('quaternion_main'):
                self.send_info(robot_name, "/main/quat", robot.get_quaternion_main(standardized=False))
            if robot.group_is_valid('rotation_main'):
                self.send_info(robot_name, "/main/rot", robot.get_rotation_main(standardized=False))
            if robot.is_valid('speed'):
                self.send_info(robot_name, "/speed", robot.get_value('speed', standardized=False))
            if robot.is_valid('steer'):
                self.send_info(robot_name, "/steer", robot.get_value('steer', standardized=False))

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

    def store_quaternion_side(self, entity_name, quat):
        self.entities[entity_name].store_quaternion_side(quat, self.get_time())

    def store_quaternion_main(self, entity_name, quat):
        # Correct euler yaw with room heading offset.
        self.entities[entity_name].store_quaternion_main(quat, self.get_time(), -self.room_heading)

    def send_info(self, entity_name, address, args):
        self.messaging.send_info("/{}{}".format(entity_name, address), args)

    def debug(self):
        import json
        self.messaging.send_info("/data", str(self.entities))

    def send_data(self):
        return
        # for robot_name in self.robots:
        #     self.messaging.send_info("/{}/pos".format(robot_name), [self.get(robot_name, 'x'), self.get(robot_name, 'y')])
