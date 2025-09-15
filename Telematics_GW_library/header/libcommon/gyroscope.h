#ifndef __GYROSCOPE_H__
#define __GYROSCOPE_H__

typedef struct _gyroscope_api_priv
{
   double x,y,z;
} gyroscope_api_priv;

int gyro_init();
int gyro_deinit();
int gyroscope_read(gyroscope_api_priv *);
int set_gyro_sampling_frequency(uint8_t);
int set_gyro_low_pass_filter(uint8_t);

#endif /* #define __GYROSCOPE_H__ */
