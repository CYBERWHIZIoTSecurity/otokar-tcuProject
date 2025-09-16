#ifndef CYBER_GPS_H
#define CYBER_GPS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "libcommon/gps.h"
#include "libcommon/gsm.h"

int doGsmActions();
int init_gps();
int deinit_gps();
int read_gps_data();

#endif // CYBER_GPS_H