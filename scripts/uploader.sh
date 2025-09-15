#!/bin/bash

LOG_DIR="/var/log/canlogs"
SENT_DIR="/var/log/canlogs/sent"

# metnonal : change these vars as needed
REMOTE_USER="XXXXX"
REMOTE_HOST="YYYY"
REMOTE_DIR="/home/iwave-tcu/canlogs"

mkdir -p "$LOG_DIR" "$SENT_DIR"

for file in "$LOG_DIR"/*.asc; do
	[ -e "$file" ] || continue

	echo "Uploading $file ..."

	scp "$file" "${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/"

	if [ $? -eq 0 ]; then
		echo "Upload successful: $file"
		mv "$file" "$SENT_DIR/"
	else
		echo "Upload failed: $file"
	fi
done