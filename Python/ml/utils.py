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

# Returns the signed difference between two angles.
def dist_angles(a1, a2):
    return math.atan2(math.sin(a1-a2), math.cos(a1-a2))

# Transforms quaternion coordinates into Euler angles (in degrees).
def quaternion_to_euler(x, y, z, w):
    import math
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

    return X, Y, Z
