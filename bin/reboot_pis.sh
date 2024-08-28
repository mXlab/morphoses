#!/bin/bash

# List of IP addresses of the remote computers
declare -a ips=("192.168.0.161")

# Username on the remote computers
username="pi"

# Loop through each IP address and issue the reboot command
for ip in "${ips[@]}"; do
    echo "Rebooting ${ip}..."
    ssh "${username}@${ip}" 'sudo reboot'
done

echo "Reboot commands sent."

