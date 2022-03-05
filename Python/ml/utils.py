import math
import numpy as np

# Map value to new range.
def map(value, fromMin, fromMax, toMin, toMax):
    # Avoids divisions by zero.
    if fromMin == fromMax:
        return (toMin + toMax) * 0.5 # dummy value
    return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin

def map01(x, in_min, in_max):
    if in_min == in_max:
        return 0.5
    return (x - in_min) / (in_max - in_min)

# Wraps an angle in degrees to be in [-180, 180].
def wrap_angle_180(angle):
    while angle > 180:
        angle -= 360
    while angle < -180:
        angle += 360
    return angle

# Returns a number in [0, 1, 2, 3] corresponding to quadrant of angle in degrees.
# 0 = front, 1 = left, 2 = right, 3 = back
def quadrant(angle):
    # If target in a 90-degree angular extent in front of the robot: front.
    if -45. < angle < 45:
        return 0

    # If target in a 90-degree angular extent at left of the robot: left.
    elif -135. < angle < -45:
        return 1

    # If target in a 90-degree angular extent at right of the robot: right.
    elif 45 < angle < 135:
        return 2

    # If target in a 90-degree angular extent behind the robot: back.
    else:
        return 3

# Returns relative heading to target from object in (x, y) with given heading (in degrees).
def target_heading(x, y, heading, target_x, target_y):
    absolute_heading_to_target = np.rad2deg(math.atan2(target_y - y, target_x - x))
    # print("{} {} {}".format(absolute_heading_to_target, heading, wrap_angle_180(heading - absolute_heading_to_target)))
    return wrap_angle_180(heading - absolute_heading_to_target)

# Returns normalized vector.
def normalize(vec):
    norm = np.linalg.norm(vec)
    if norm == 0:
        return vec
    else:
        return vec / norm

# Returns linear interpolation between two colors.
def lerp_color(t, from_color, to_color):
    color = []
    for i in range(3):
        color.append( (1-t) * from_color[i] + t * to_color[i])
    return color

# Returns the signed difference between two angles.
def dist_angles(a1, a2):
    return math.atan2(math.sin(a1-a2), math.cos(a1-a2))

# Transforms quaternion coordinates into Euler angles (in degrees).
def quaternion_to_euler(x, y, z, w, zOffset=0):
    # Roll.
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + y * y)
    X = math.degrees(math.atan2(t0, t1)) # in [-180, 180]
    # Pitch.
    t2 = +2.0 * (w * y - z * x)
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    Y = math.degrees(math.asin(t2)) # in [-90, 90]
    # Yaw.
    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (y * y + z * z)
    Z = math.degrees(math.atan2(t3, t4)) # in [-180, 180]

    # Adjust yaw according to offset.
    Z = wrap_angle_180(Z + zOffset)

    return X, Y, Z

# Deprecated: Returns the robot's instantaneous orientation.
def compute_orientation(delta_data, speed):
    forward = np.sign(speed)
    return np.angle(np.complex(forward*delta_data[0], forward*delta_data[1]))

# Returns the robot's instantaneous direction to target, discretised as 0=front, 1=right, 2=left, 3=back. Optional: Roughly plots relevant vectors and past robot positions.
def compute_direction_to_target(data, delta_data, target_position_state, speed, plot):
    print('pos', [data[0], data[1]])

    # Computer vector for robot instantaneous direction.
    forward = np.sign(speed)
    delta_pos = [forward*delta_data[0], forward*delta_data[1]] # invert according to current speed
    print("velocity: ({}, {}) speed: {} orientation: {}".format(delta_pos[0], delta_pos[1], speed, np.degrees(compute_orientation(delta_data, speed))))

    # Compute vector from robot to target state.
    delta_robot_to_target = [target_position_state[0] - data[0], target_position_state[1] - data[1]]

    # Normalize vectors.

    delta_pos = delta_pos / np.linalg.norm(delta_pos)
    delta_robot_to_target = delta_robot_to_target / np.linalg.norm(delta_robot_to_target)

    # Compute relative angle.
    dot_product = np.dot(delta_pos, delta_robot_to_target)
    angle = np.arccos(dot_product)
    angle = np.rad2deg(angle)

    # If target in a 90-degree angular extent in front of the robot: front.
    if -45. < angle < 45:
        state = 0.

    # If target in a 90-degree angular extent at right of the robot: right.
    elif -135. < angle < -45:
        state = 1. / 3.

    # If target in a 90-degree angular extent at left of the robot: left.
    elif 45 < angle < 135:
        state = 2. / 3.

    # If target in a 90-degree angular extent behind the robot: back.
    else:
        state = 1.

    # Optional: Roughly plots relevant vectors and past robot positions.
    if plot:
        origin = [data[0]], [data[1]]

        plt.subplot(121)
        plt.cla()
        plt.xlim(0., 1.)
        plt.ylim(0., 1.)
        plt.quiver(*origin, [delta_pos[0], delta_robot_to_target[0]], [delta_pos[1], delta_robot_to_target[1]], color=['r','b'])
        plt.scatter(target_position_state[0], target_position_state[1])
        plt.xlabel('state: ' + str(state) + ', ' + 'angle: ' + str(angle))

        plt.subplot(122)
        plt.xlim(0., 1.)
        plt.ylim(0., 1.)
        plt.scatter(data[0], data[1], s=1, c='#000000')
        #plt.scatter(target_position_state[0], target_position_state[1])
        plt.draw()
        plt.pause(0.001)

    return state