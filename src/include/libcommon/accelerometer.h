#ifndef __ACCELEROMETER_H__
#define __ACCELEROMETER_H__

#include <math.h>

typedef struct _accelerometer_api_priv
{
	int fd;
	double x,y,z,acc;
} accelerometer_api_priv;

int acc_init();
int acc_deinit();
int accelerometer_read(accelerometer_api_priv *adata);
int set_acc_sampling_frequency(uint8_t);
int set_acc_low_pass_filter(uint8_t);
int set_acc_wakeup_threshold(uint8_t);
int acc_temp_read( float_t *temperature );

#endif /* #define __ACCELEROMETER_H__ */
