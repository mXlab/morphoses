import numpy as np
from sklearn.preprocessing import MinMaxScaler

MIN_SPEED = -15
MAX_SPEED = 15

MIN_STEERING = -45
MAX_STEERING = 45

MIN_POSITION = -25
MAX_POSITION = 25

MIN_ANGLE = -180
MAX_ANGLE = -180

# Transforms quaternion coordinates into Euler angles (in degrees).
def quaternion_to_euler(x, y, z, w):
    import math
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + y * y)
    X = math.degrees(math.atan2(t0, t1))

    t2 = +2.0 * (w * y - z * x)
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    Y = math.degrees(math.asin(t2))

    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (y * y + z * z)
    Z = math.degrees(math.atan2(t3, t4))

    return X, Y, Z

# Reads the dataset, standardizes it and returns the inputs, targets, and corresponding MinMaxScaler objects.
# Optional argument "bins" will convert the targets into classes.
def preprocess_data(dataset, prune_experiments = False, bins = None):

    # Prune rows at beginning of experiments where speed and steering are too small.
    if prune_experiments:
        pruned_dataset = []
        current_exp = dataset[0][0]
        current_exp_has_started = False
        for row in dataset:
            # Check if we just changed experiment.
            exp = row[0]
            if (exp != current_exp):
                current_exp = exp
                current_exp_has_started = False

            # Check if the ball has started to move.
            if abs(row[8]) >= 5 or abs(row[9]) >= 5:
                current_exp_has_started = True
            # If ball has started to move then add row
            if current_exp_has_started:
                pruned_dataset.append(row)

        # Set dataset to pruned version.
        dataset = np.array(pruned_dataset)

    # Create input matrix X.
    pos_X = np.array(dataset[:, 2:4], dtype='float64')
    quat_X = np.array(dataset[:, 4:8], dtype='float64')
    euler_X = np.matrix([quaternion_to_euler(q[0], q[1], q[2], q[3]) for q in quat_X])

    # Join blocks.
    X = np.concatenate((pos_X, quat_X, euler_X), axis=1)

    # Normalize X.
    scalerX = MinMaxScaler()
    scalerX.fit(X)
    X = scalerX.transform(X)

    # Create target matrix Y.
    speed_y = np.array(dataset[:, 8], dtype='float64')
    steering_y = np.array(dataset[:, 9], dtype='float64')

    # Join blocks.
    Y = np.column_stack((speed_y, steering_y))

    # Normalize Y.
    scalerY = MinMaxScaler()
    scalerY.fit(Y)
    Y = scalerY.transform(Y)

    # Classification.
    # This has been UNTESTED.
    if bins != None:
        if (type(bins) is tuple or type(bins) is list):
            n_bins_speed = bins[0]
            n_bins_steering = bins[1]
        else:
            n_bins_speed = n_bins_steering = bins

        # Generate bins.
        bins_speed    = np.histogram_bin_edges(speed_y, n_bins_speed)
        bins_steering = np.histogram_bin_edges(steering_y, n_bins_steering)

        # Convert to classes.
        speed_y = np.digitize(speed_y, bins_speed)
        steering_y = np.digitize(steering_y, bins_steering)

        Y = np.column_stack((speed_y, steering_y))

    return X, Y, scalerX, scalerY

# def standardize(value, min, max):
#     return (value - min) / (max - min)
#
# def standardize_positions(pos):
#     return standardize(pos, MIN_POSITION, MAX_POSITION)
#
# def standardize_euler(euler):
#     return standardize(euler, MIN_ANGLE, MAX_ANGLE)
#
# def standardize_speed(speed):
#     return standardize(speed, MIN_SPEED, MAX_SPEED)
#
# def standardize_steering(steering):
#     return standardize(steering, MIN_STEERING, MAX_STEERING)

# Standardizes an input point using its scaler.
def standardize(datapoint, scaler):
    return scaler.transform([datapoint])