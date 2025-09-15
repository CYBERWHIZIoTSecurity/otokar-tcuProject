#ifndef __I_BATTERY_H__
#define __I_BATTERY_H__

int i_battery_init();
int i_battery_get_health();
int i_battery_get_voltage( double * );
int i_get_battery_status( int *b_chrg_status );
int battery_connect_config(int con_status);
int battery_charge_state_config(int state);
int battery_charge_state_config(int state);
int battery_connect_config(int con_status);
int get_power_source();
int i_get_battery_temp( int *b_temp_status );

#endif /* __I_BATTERY_H__ */
