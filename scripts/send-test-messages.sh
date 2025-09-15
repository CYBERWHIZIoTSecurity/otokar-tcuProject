#!/bin/bash

# script is using for sending dummy messages on can0 interface
# author: metin.onal@cyberwhiz.co.uk

IFACE="can0"
MSG="123#DEADBEEF"

while true; do
	cansend $IFACE $MSG
	usleep 1000
done