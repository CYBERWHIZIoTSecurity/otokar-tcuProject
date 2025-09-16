#ifndef CYBER_CANBUS_H
#define CYBER_CANBUS_H

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <unistd.h>
#include "libcommon/can.h"

#define CAN_INTERFACE			"can1"
#define CAN_BITRATE			500000
#define CAN_READ_TIMEOUT_ERR_CODE	0x9001000a
#define CAN_LOG_FILE_SIZE_LIMIT		(1 * 1024 * 1024) // 1 MB

#endif // CYBER_CANBUS_H