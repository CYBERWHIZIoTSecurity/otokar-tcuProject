#ifndef __ACCELEROMETER_H__
#define __ACCELEROMETER_H__

#define ENABLE  		1
#define DISABLE 		0

#define ACC_I2C_NO		3

#define ACC_MAIN_PATH 		"/sys/bus/iio/devices/iio:device"
#define ACC_SUB_PATH_SCAN   	"/scan_elements"
#define ACC_SUB_PATH_BUF    	"/buffer"
#define ACC_EVENT_NAME		"accel"
#define ACC_INTERRUPT_X_AXIS 	"in_accel_x_en"
#define ACC_INTERRUPT_Y_AXIS 	"in_accel_y_en"
#define ACC_INTERRUPT_Z_AXIS 	"in_accel_z_en"
#define ACC_BUFFER_ENABLE    	"enable"
#define CRASH_THRESHOLD		1
#if 0
#define ACC_SLAVE_ADDR		"6A"
#define SENSOR_CTRL_REG1	"10"
#define SENSOR_CTRL_REG8	"17"
#define ACC_THRESHOLD_REG	"59"
#define SENSOR_INT_DUR2		"5A"
#define SENSOR_WAKE_UP_THS	"5B"
#define SENSOR_MD1_CFG		"5E"
#define SENSOR_TAP_CFG		"58"
#define SENSOR_INT1_CTRL	"0D"
#endif
#define ACC_SLAVE_ADDR          0x6a
#define SENSOR_CTRL_REG1        0x10
#define SENSOR_CTRL_REG8        0x17
#define ACC_THRESHOLD_REG       0x59
#define SENSOR_INT_DUR2         0x5A
#define SENSOR_WAKE_UP_THS      0x5B
#define SENSOR_MD1_CFG          0x5E
#define SENSOR_TAP_CFG          0x58
#define SENSOR_INT1_CTRL        0x0D

typedef struct _accelerometer_api_priv
{
    int fd;
    double x,y,z,acc;
} accelerometer_api_priv;

/* global declaration */
int acc_init();
int acc_deinit();
int sensor_init();
void accelerometer_read_thread (void);
int set_acc_sampling_frequency(uint8_t);
int set_acc_low_pass_filter(uint8_t);
int set_acc_wakeup_threshold(uint8_t);
int acc_sensitivity();
int accelerometer_read (accelerometer_api_priv *acc);

#endif /* #define __ACCELEROMETER_H__ */
