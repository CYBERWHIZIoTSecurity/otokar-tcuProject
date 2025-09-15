#ifndef __MCU_I2C_COMMON_H__
#define __MCU_I2C_COMMON_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

typedef struct i2c_cmd_frame_struct
{
	uint8_t i2c_addr;
	uint8_t reg;
	uint8_t command_id;
	uint8_t length;
	uint8_t *data;
} i2c_cmd_frame;

typedef struct i2c_cmd_decode_struct
{
	uint8_t response_id;
	uint8_t err_code[4];
	uint8_t length;
	uint8_t *data;
} i2c_cmd_decode;

/* Sleep Wakeup Source Bitmask */
#define CPU_MCU_TIMER_WAKEUP_REQUEST_BIT_MASK	0x80
#define CPU_MCU_IGN_WAKEUP_REQUEST_BIT_MASK	0x40
#define CPU_MCU_ACC_WAKEUP_REQUEST_BIT_MASK	0x20
#define CPU_MCU_CAN_WAKEUP_REQUEST_BIT_MASK	0x10
#define CPU_MCU_RTC_WAKEUP_REQUEST_BIT_MASK	0x08

/* CPU to MCU command request APIs */
int MCU_sleep_mode( uint8_t sleep_mode, uint8_t wakeup_source );
int MCU_Wakeup_Source_Request( uint8_t *wake_source );
int MCU_FW_Version_Read_Request( char *mcu_fw_version );
int MCU_ADC_Read_Request( int adc, float *voltage );

#endif /* __MCU_I2C_COMMON_H__ */
