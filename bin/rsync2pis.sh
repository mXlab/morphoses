#!/bin/bash

# Directory to upload
local_file="$(pwd)/Processing/MorphosesDisplay/"

# Remote directory where to upload the directory
remote_dir="/home/pi/morphoses/Processing/MorphosesDisplay"

# List of IP addresses of the remote computers
declare -a ips=("192.168.0.161" "192.168.0.171" "192.168.0.181")

# Username on the remote computers
username="pi"
password="BouleQuiRoule"

# Loop through each IP address and use rsync to upload the directory
for ip in "${ips[@]}"; do
    echo "Uploading to ${ip}..."
    rsync -avz -e "sshpass -p ${password} ssh -o StrictHostKeyChecking=no" "${local_file}" "${username}@${ip}:${remote_dir}"
done

echo "Upload completed."

