#ifndef __MAGNETOMETER_H__
#define __MAGNETOMETER_H__

typedef struct _magnetometer_api_priv
{
	int fd;
	double x,y,z;
} magnetometer_api_priv;

int mag_init();
int mag_deinit();
int magnetometer_read(magnetometer_api_priv *);
int set_mag_sampling_frequency(uint8_t );

#endif /* #define __MAGNETOMETER_H__ */
