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
    total = 0; records = []
    for line in csv_input_file:
        total += 1
        records.append(line)
    csv_reader = csv.reader(records)
    # discard the header row with column names
    next(csv_reader)

    # Create CSV file.
    csv_output_file = open(args.output_file, "w")
    csv_writer = csv.DictWriter(
        csv_output_file, fieldnames=['id', 'time', 'x', 'y', 'qx', 'qy', 'qz', 'qw', 'steering', 'speed'])
    csv_writer.writeheader()

    # Create *n_rotations* versions of original data.
    data = [[] for i in range(n_rotations)]

    print(f"transforming {total -1} records")
    alerted = 0
    for counter, row in enumerate(csv_reader):
        percent_complete = math.floor(counter / total * 100)
        if (percent_complete > alerted * 10):
            print(f"{percent_complete} percent complete")
            alerted = alerted + 1

        # Get positional and quaternion values.
        exp_id, time, x, z, qx, qy, qz, qw, speed, steering = row
        #print(exp_id, time, x, z, qx, qy, qz, qw, speed, steering)
        pos = np.array([x, 0, z]).reshape((3, 1)).astype(float)
        quat = np.array([qx, qy, qz, qw]).astype(float)

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
            # print([ new_pos[0], new_pos[2], new_quat[0], new_quat[1], new_quat[2], new_quat[3]])
            data[i].append([exp_id, time, new_pos[0], new_pos[2], new_quat[0],
                            new_quat[1], new_quat[2], new_quat[3], speed, steering])

    # Merge all data in a single CSV file
    print("writing file")
    for version in data:
        for row in version:
            csv_writer.writerow({
                "id": row[0],
                "time": row[1],
                "x": row[2],
                "y": row[3],
                "qx": row[4],
                "qy": row[5],
                "qz": row[6],
                "qw": row[7],
                "steering": row[8],
                "speed": row[9]
            })
