#!/usr/bin/env python3

# this script is using for playing back a CAN log file in CSV format
# in ubuntu machine
# author : metin.onal@cyberwhiz.co.uk

import time
import can
import csv

bus = can.interface.Bus(channel="can0", bustype="socketcan")

log_file = "sample-log-file.csv"

start_ts = None
start_time = None

with open(log_file) as f:
	reader = csv.DictReader(f)
	for row in reader:
		try:
			ts = int(row["Time Stamp"]) / 1e6
			canid = int(row["ID"], 16)
			dlc = int(row["LEN"])
			is_extended = row["Extended"].lower() == "true"

			data_bytes = []
			for i in range(1, dlc+1):
				key = f"D{i}"
				if row[key]:
					data_bytes.append(int(row[key], 16))

			if start_ts is None:
				start_ts = ts
				start_time = time.time()

			now = time.time()
			elapsed = now - start_time
			wait_time = (ts - start_ts) - elapsed
			if wait_time > 0:
				time.sleep(wait_time)

			msg = can.Message(arbitration_id=canid, data=bytes(data_bytes), is_extended_id=is_extended)
			bus.send(msg)
			print(f"Sent t={ts:.6f}s ID=0x{canid:X} DLC={dlc} Data={' '.join(f'{b:02X}' for b in data_bytes)}")
		except Exception as e:
			print("Skip line due to parse error:", e, row)