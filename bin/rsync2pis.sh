#!/bin/bash

# Directory to upload
local_file="$(pwd)/MorphosesTitleDisplay/"

# Remote directory where to upload the directory
remote_dir="/home/pi/morphoses/MorphosesTitleDisplay"

# List of IP addresses of the remote computers
declare -a ips=("192.168.0.161")

# Username on the remote computers
username="pi"
password="BouleQuiRoule"

# Loop through each IP address and use rsync to upload the directory
for ip in "${ips[@]}"; do
    echo "Uploading to ${ip}..."
    rsync -avz -e "sshpass -p ${password} ssh -o StrictHostKeyChecking=no" "${local_file}" "${username}@${ip}:${remote_dir}"
done

echo "Upload completed."

