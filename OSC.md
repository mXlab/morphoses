# OSC commands

## Main + IMU Boards

Main board:
* IP: 192.168.0.100
* Port (to main board): 8765
* Port (from main board): 8766

IMU board:
* IP: 192.168.0.101
* Port (to IMU board): 8765
* Port (from IMU board): 8767

### Receive

| Boards | Message        | Types | Explanation |
| ------ | -------------- | ----- | ----------- |
| Main   | ```/motor/1``` | i     | Controls speed/pitch motor with values in [-255, 255] |
| Main   | ```/motor/2``` | i     | Controls steer/roll motor with values in [-90, 90] (corresponding to angle) |
| Both   | ```/bonjour``` |       | Connection message sent to the board; upon receiving it the board will update its ```destIP``` address to send its messages to the right place. |

### Send

| Boards | Message              | Types | Explanation |
| ------ | -------------------- | ----- | ----------- |
| Main   | ```/motor/1/ticks``` | i     | Total number of ticks of speed/pitch motor (from encoder) |
| Main   | ```/motor/2/ticks``` | i     | Total number of ticks of steer/roll motor (from encoder) |
| Both   | ```/quat```          | ffff  | Quaternion values (from IMU) |
| Both   | ```/euler```         | fff   | Euler angles (from IMU) |
| Main   | ```/main/bonjour```  |       | Sent by Main board in response to the ```/bonjour``` message to indicate reception.  |
| IMU    | ```/imu/bonjour```   |       | Sent by IMU board in response to the ```/bonjour``` message to indicate reception.  |
| Both    | ```/imu/i2c/error``` |       | Sent if the IMU has an I2C problem |
| Both    | ```/imu/i2c/ok```    |       | Sent if the IMU is ok (no I2C problems) |


