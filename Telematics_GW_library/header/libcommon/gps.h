#ifndef __GPS_H_
#define __GPS_H_

/*RMC*/

#define TIME_STAMP          1
#define RMA_FIX_STATUS      2
#define RMA_CUR_LATITUDE    3
#define RMA_HEMISPHERE      4
#define RMA_CUR_LONGITUDE   5
#define RMA_GREENWICH       6
#define RMA_KNOT_SPEED      7
#define RMA_DIRECTION       8
#define GPS_NODE        "/dev/ttyUSB1"

struct gps_rmc_t
{
        unsigned int hour;
        unsigned int minutes;
        unsigned int seconds;
        double latitude;
        double longitude;
        double speed;
        double direction;
        char nmea[200];
        int gps_valid;
        int gps_init_fix;
};

struct gps_pkt
{
        struct gps_rmc_t gps_rmc;
};

int gps_init();
int gps_deinit();
int agps_init();
int get_gps_data(char * nmea, size_t * g_nbytes, char *recv_data, int length);

#endif
