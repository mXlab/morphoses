import numpy as np
import math
import time

import mp.preprocessing as mpp

# Returns the signed difference between two angles.
def dist_angles(a1, a2):
    return math.atan2(math.sin(a1-a2), math.cos(a1-a2))

# Returns the difference between current and previous datapoints.
def delta(data, prev_data):
    two_pi = 2*math.pi
    d = data - prev_data
    d[6] = dist_angles(data[6]*two_pi, prev_data[6]*two_pi)
    d[7] = dist_angles(data[7]*math.pi, prev_data[7]*math.pi)
    d[8] = dist_angles(data[8]*two_pi, prev_data[8]*two_pi)
    return d

def map(x, in_min, in_max, out_min, out_max):
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def map01(x, in_min, in_max):
    return (x - in_min) / (in_max - in_min)

# Holds one data point, allowing to compute its delta and standardization.
class Data:
    def __init__(self, value=0,
                 min_value=-1, max_value=+1,
                 max_change_per_second=0.1,
                 is_angle=False):
        self.value = value
        self.prev_value = value
        self.delta_value = 0
        self.stored_value = value
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

    def __repr__(self):
        return str( "s:{} v:{} d:{}".format(self.stored_value, self.value, self.delta_value))

class EntityData:
    def __init__(self, labels):
        self.data = {}
        for l in labels:
            self.data[l] = Data()

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

    def __repr__(self):
        return str(self.data)

class RobotData(EntityData):
    def __init__(self):
        super().__init__(['x', 'y',
                          'qx', 'qy', 'qz', 'qw', 'rx', 'ry', 'rz',
                          'mqx', 'mqy', 'mqz', 'mqw', 'mrx', 'mry', 'mrz'])

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

    def store_quaternion(self, quat, t):
        self.store(['qx', 'qy', 'qz', 'qw'], quat, t)
        rx, ry, rz = mpp.quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['rx', 'ry', 'rz'], [rx, ry, rz], t)

    def store_quaternion_main(self, quat, t):
        self.store(['mqx', 'mqy', 'mqz', 'mqw'], quat, t)
        rx, ry, rz = mpp.quaternion_to_euler(quat[0], quat[1], quat[2], quat[3])
        self.store(['mrx', 'mry', 'mrz'], [rx, ry, rz], t)

class ThingData(EntityData):
    def __init__(self):
        super().__init__(['x', 'y'])

    def store_position(self, position, t):
        self.store(['x', 'y'], position, t)

class World:
    def __init__(self, settings):
        self.start_time = time.time()
        self.entities = {}
        for robot in settings['robots']:
            robot_name = robot['name']
            self.entities[robot_name] = RobotData()
        for thing in settings['things']:
            thing_name = thing['name']
            self.entities[thing_name] = ThingData()

    def get(self, entity_name, variable, standardized=True):
        if variable.startswith('d_'):
            var = variable[2:] # remove the 'd_' part
            return self.entities[entity_name][var].get_delta(standardized)
        else:
            return self.entities[entity_name][variable].get(standardized)

    def update(self):
        for entity in self.entities.values():
            entity.update()

    def get_time(self):
        return time.time() - self.start_time

    def store_position(self, entity_name, pos):
        self.entities[entity_name].store_position(pos, self.get_time())

    def store_quaternion(self, entity_name, quat):
        self.entities[entity_name].store_quaternion(quat, self.get_time())

    def store_quaternion_main(self, entity_name, quat):
        self.entities[entity_name].store_quaternion_main(quat, self.get_time())

    def debug(self):
        print(self.entities)