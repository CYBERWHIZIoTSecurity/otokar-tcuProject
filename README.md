# CAN Bus Implementation

This projects is development environment of TCU Project which will be held between CyberWhiz IoT Cyber Security & OTOKAR.

## Folder Descriptions

### build
    * this folder is output folder. when you run 'make' command from /src folder, output files will be occured in here

### docker
    * this folder is included with dockerfile, iwave-toolchain and sample docker run script for local usage.

### docs
    * folder is included with sample user manual document which is coming from iwave manufacturer company.

### scripts
    * project development environment is created with a ubuntu machine & iwave development card. so, there is a need to send some scripts to ubuntu machine for testing / simulating.

    * play_log_file.py -> this script simulates the sample-log-file.csv file.
    * sample-log-file.csv -> this is the log file which is taken from e-kent2 bus from one ECU.
    * send-test-messages.sh -> this scripts sends sample can messages to canbus line for every seconds. it is written for proving canbus line.
    * uploader.sh -> scripts can be use for sending taken log files to cloud.

### src
    * canbus-interface.c -> this is the main source code. as today, there is only one c file which manages everything as 15 of september. however, it needs to be divided for micro-management. this implementation is written for beginning.

    * include/libcommon -> this folder is coming from iwave manufacturer company. header files for lib usage.

    * Makefile -> as 15 september, there is only one rule which is tcu-app. so, you can build the project with using 'make tcu-app' command from terminal.

### Telematics GW Lib
    * included with .so file which provides to us that using manufacturer specific lib functions.

### temp
    * unusing. but included with sample makefile and source codes. can be deleted.

### blf log file
    * sample log file which is taken blf file extension.

## Project details
    * You can access to ubuntu machine which has static ip with, " ssh cyberwhiz@78.186.198.244 -p 22 " after accessing to ssh connection, can be accessed to iwave telemetry card from local connection : "ssh root@192.168.1.127 -p 22"

    * for cyberwhiz user, password is 'cyberwhiz123'
    * for root user, password is 'iWavesys123'

## Notes
    * this is an initial version of readme file, it will be updated for feature updates (15 Sep 2025).
