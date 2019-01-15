# Workspaces

There are several Python virtual environments which contain the scripts.

To setup the virtual environment:

1. Enter the virtual env folder
2. Run `python -m venv .`
3. Run `pip install -r requirements.txt`

To use a virtual environment:

1. enter the virtual env folder
2. run the following command:

| Shell     | Command                      |
| --------- | ---------------------------- |
| `bash`    | `source ./bin/activate`      |
| ----      | ----                         |
| `fish`    | `source ./bin/activate.fish` |
| ----      | ----                         |
| `pwsh`    | `./bin/Activate.ps1`         |
| ---       | ----                         |
| `cmd.exe` | `./bin/activate.bat`         |

## Unity Interop Scripts

These scripts are in the folder `unity_interop` and help work with the Unity project in `MorphosisBall`.

The script `mp_unity_recieve_csv.py` can be used to populate a csv file with the output of OSC packets from the unity
simulation.

The script `mp_unity_send_csv.py` sends data back to the unity simulation from a seed file, over time. WIP

To create more data from a small amount of trials `mp_unity_transform_csv.py` can be run with the existing file.

## osc utils

Scripts in the `osc_utils` workspace can be run to test osc communication.

## Other tools

The following are other tools that may help when working with this codebase.

- `pyliblo` - a set of tools to work with the `liblo` OSC library. Includes the `send_osc` and `dump_osc` commands.
  - On Ubuntu: `apt install pyliblo-utils`
-
