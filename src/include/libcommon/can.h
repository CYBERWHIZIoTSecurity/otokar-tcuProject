#ifndef CAN_H
#define CAN_H

#include <linux/can.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>


#define CAN_INTERFACE			"can1"
#define CAN_BITRATE			500000
#define CAN_READ_TIMEOUT_ERR_CODE	0x9001000a
#define CAN_LOG_FILE_SIZE_LIMIT		(1 * 1024 * 1024) // 1 MB

int can_init(const char *, int);
int can_fd_init(const char *,int bitrate, int dbitrate, int txqueuelen);
int set_can_mask_and_filter(uint32_t *mask, uint32_t *filter, int no_of_filter);
int can_write(char *, char *);
int can_read(char *, struct canfd_frame *);
int can_deinit(const char *);


#endif /* CAN_H */
