/*
	An application for reading CANBUS messages.
	author: metin.onal@cyberwhiz.co.uk
*/

#include "include/libcommon/can.h"

static FILE *logfile = NULL;
static struct timespec ts_start;
static int fileIndex = 0;

void rotateLogFile()
{
	if (logfile != NULL)
	{	
		fclose(logfile);
		logfile = NULL;
	}

	int ret = 0;
	char filename[64];
	ret = snprintf(filename, sizeof(filename), "canlog_%03d.asc", fileIndex++);
	if (ret < 0 || ret >= sizeof(filename))
	{
		printf("Log file name generation failed\n");
		exit(1);
	}

	logfile = fopen(filename, "w");
	if (logfile == NULL)
	{
		printf("Log file open failed: %s\n", filename);
		exit(1);
	}

	time_t now = time(NULL);
	fprintf(logfile, "date,%s", ctime(&now));
	fprintf(logfile, "base hex timestamps absolute\n");
	fprintf(logfile, "no interval events logged\n");
	fflush(logfile);

	clock_gettime(CLOCK_REALTIME, &ts_start);
}

int logFileInit(const char *filename)
{
	logfile = fopen(filename, "w");
	if (logfile == NULL)
	{
		printf("Log file open failed: %s\n", filename);
		return -1;
	}

	clock_gettime(CLOCK_REALTIME, &ts_start);

	time_t now = time(NULL);
	fprintf(logfile, "date,%s", ctime(&now));
	fprintf(logfile, "base hex timestamps absolute\n");
	fprintf(logfile, "no interval events logged\n");
	fflush(logfile);

	return 0;
}

void logFileLogMessage(uint32_t id, const char *dir, int channel,
	uint8_t dlc, const uint8_t *data)
{
	if (logfile == NULL)
		return;
	
	struct timespec ts_now;
	clock_gettime(CLOCK_REALTIME, &ts_now);
	double timestamp = (ts_now.tv_sec - ts_start.tv_sec) +
		(ts_now.tv_nsec - ts_start.tv_nsec) / 1e9;
	fprintf(logfile, "%.6f %d %X %s d %d", timestamp, channel, id, dir, dlc);
	for (int i = 0; i < dlc; i++)
	{
		fprintf(logfile, " %02X", data[i]);
	}
	fprintf(logfile, "\n");
	fflush(logfile);

	long pos = ftell(logfile);
	if (pos >= CAN_LOG_FILE_SIZE_LIMIT)
	{
		rotateLogFile();
	}
}

void logFileDeinit(void)
{
	if (logfile != NULL)
	{
		fclose(logfile);
		logfile = NULL;
	}
}

void canRxCallback(const struct canfd_frame *frame, int channel)
{
	const char *dir = "Rx";
	logFileLogMessage(frame->can_id, dir, channel, frame->len, frame->data);
}

int main(void)
{
	int ret = 0, s = 0;
	struct canfd_frame frame;

	printf("CAN interface init: %s, bitrate=%d\n", CAN_INTERFACE, CAN_BITRATE);

	ret = can_init(CAN_INTERFACE, CAN_BITRATE);
	if (ret != 0)
	{
		printf("CAN init failed, ret=0x%x\n", ret);
		return -1;
	}
	printf("CAN init success\n");

	char filename[64];
	ret = snprintf(filename, sizeof(filename), "canlog_%03d.asc", fileIndex++);
	if (ret < 0 || ret >= sizeof(filename))
	{
		printf("Log file name generation failed\n");
		exit(1);
	}

	ret = logFileInit(filename);
	if (ret != 0)
	{
		logFileDeinit();
		printf("Log file init failed\n");
		return -1;
	}
	printf("Log file init success\n");

	while (1)
	{
		memset(&frame, 0, sizeof(frame));

		ret = can_read(CAN_INTERFACE, &frame);
		if (ret > 0)
		{
			logFileLogMessage(frame.can_id, "Rx", 1, frame.len, frame.data);
		}
		else if (ret < 0)
		{
			if (ret == CAN_READ_TIMEOUT_ERR_CODE)
			{
				continue;
			}
			else
			{
				printf("CAN read error, ret=0x%x\n", ret);
				break;
			}
		}
		usleep(1000);
	}

	logFileDeinit();

	ret = can_deinit(CAN_INTERFACE);
	if (ret != 0)
	{
		printf("CAN deinit failed, ret=0x%x\n", ret);
		return -1;
	}

	printf("CAN deinit success\n");
	return 0;
}

