#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <stdint.h>

#define DISABLE                 0
#define ENABLE                  1

#define CAN0			"can0"
#define CAN1			"can1"
#define CAN2			"can2"
#define CAN3			"can3"
#define CAN4			"can4"

extern int rs232_fd;
extern int rs485_fd;
int rs485_mode;

/* Common APIs */
char *version_read();
int init( int network_enable );
int deinit( );
void get_time(char *);
int get_mac_address(char *interface,char *mac_address);
int get_cpu_id(char *cpuid, int cpuid_len);
int ntp_server_update();
int led_enable();
int led_disable();
int check_adc2_voltage(double *);
int check_adc1_voltage(double *);
int check_ext_voltage(double *);
int restart_device();
int i2c_write(int, uint8_t, uint8_t, uint8_t );
int i2c_read(int, uint8_t, uint8_t, uint8_t *);
int push_device_to_sleep();
int ignition_pin_status();
int ign_pin_status_check_enable();
int ign_pin_status_check_disable();

/* WiFi-BT Related APIs */
int wifi_init(int mode);
int wifi_deinit();
int ble_init();
int ble_deinit();

/* Ethernet Related APIs */
int eth_init(char* iface);
int eth_deinit(char* iface);

/* Sleep Wakeup Related APIs */
int disable_all_wakeup_sources();
int config_timer_wakeup( int option, int timer );
int config_rtc_wakeup( int option, int timer );
int config_ignition_wakeup( int option );
int config_acc_wakeup( int option );
int config_can_wakeup( char *can_name, int option );
int config_sms_wakeup( int option );
int config_mcu_wakeup( int option );

/* RS232 Related APIs */
int rs232_init(int baudrate);
int rs232_deinit();
int rs232_read(char *buf, long int sz);
int rs232_write( char *buf, size_t sz);

/*RS485 Related APIs */
int rs485_init(int baudrate);
int rs485_deinit();
int rs485_read(char *buf, long int sz, int mode);
int rs485_write(char *buf, size_t sz);

/* Digital Input and Output */
int read_digital_in(int din, int *state);
int write_digital_out(int dout, int state);


#endif /* __COMMON_H__ */
