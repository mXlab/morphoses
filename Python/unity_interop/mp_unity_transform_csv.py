import argparse
import math
import csv

from pythonosc import dispatcher
from pythonosc import osc_server
import numpy as np
from transforms3d.derivations.eulerangles import y_rotation
from transforms3d.quaternions import quat2mat, mat2quat, qmult, axangle2quat


def handle_data(unused_addr, x, z, qx, qy, qz, qw):
    csv_writer.writerow([x, z, qx, qy, qz, qw])


if __name__ == "__main__":
    # Create parser
    parser = argparse.ArgumentParser()

    parser.add_argument("input_file", type=str, help="CSV input file")
    parser.add_argument("output_file", type=str, help="CSV output file")
    parser.add_argument("-n", "--n-rotations", type=int, default=12,
                        help="The number of rotations (including the base).")

    # Parse arguments.
    args = parser.parse_args()

    n_rotations = args.n_rotations

    # Open CSV input file.
    csv_input_file = open(args.input_file, "r")
    csv_reader = csv.reader(csv_input_file, quoting=csv.QUOTE_NONNUMERIC)

    # Create CSV file.
    csv_output_file = open(args.output_file, "w")
    csv_writer = csv.writer(csv_output_file)
#    csv_writer = csv.DictWriter(csv_file, fieldnames=['x', 'y', 'qx', 'qy', 'qz', 'qw'])
#    csv_writer.writeheader()

    # Create *n_rotations* versions of original data.
    data = [[] for i in range(n_rotations)]

    for row in csv_reader:
        # Get positional and quaternion values.
        x, z, qx, qy, qz, qw = row
        pos = np.array([x, 0, z]).reshape((3, 1))
        quat = np.array([qx, qy, qz, qw])

        # Apply transformations
        for i in range(n_rotations):
            angle = i / n_rotations * 2 * np.pi
            rot_quat = axangle2quat([0, 1, 0], angle)
            rot = quat2mat(rot_quat)

            # Rotate object.
            new_pos = np.dot(rot, pos).reshape(3)
            #rot = np.array(rot).astype(np.float64)
            new_quat = qmult(quat, rot_quat)

            # print(new_pos)
            # print(new_quat)

            # Create new row.
#            print([ new_pos[0], new_pos[2], new_quat[0], new_quat[1], new_quat[2], new_quat[3]])
            data[i].append([new_pos[0], new_pos[2], new_quat[0],
                            new_quat[1], new_quat[2], new_quat[3]])

    # Merge all data in a single CSV file
    for version in data:
        for row in version:
            csv_writer.writerow(row)
