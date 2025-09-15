#!/bin/bash

docker run --platform linux/amd64 -it --rm \
	-v ~/repositories/tcu-dev:/tcu-dev \
	tcu-docker "$@"