#ifndef __GSM_HEADER__
#define __GSM_HEADER__
#include <semaphore.h>

#define GSM_GPS_PORT		"/dev/ttyUSB0"
#define GSM_LOG_PORT		"/dev/ttyUSB1"
#define GSM_AT_PORT		"/dev/ttyUSB2"
#define GSM_CONNECTION_PORT	"/dev/ttyUSB3"

int gsm_at_cmd(char *, char *, int, int);
int get_gsm_imei(char *, int);
int check_gsm_nw_connection();
int check_network_connection();
int set_gsm_flight_mode_on();
int set_gsm_flight_mode_off();
int gsm_modem_on(char*, int);
int gsm_modem_off();
int get_gsm_sim_status(int *sim_status_val);
int get_gsm_sim_iccid(char *, int);
int set_gsm_network_mode(int);
int get_gsm_signal_strength(char *, int);
int get_gsm_nw_reg(char *, int, char *, int);
int check_gsm_modem_status();
int GSM_set_to_message_init( );
int unread_message(char *msg_buf, int length, int max_resp_time );
int read_message(char *msg_buf, int length, int max_resp_time );
int send_sms( char *msg_response, char *sender_number, int max_resp_time );
int delete_message( int index, int max_resp_time );
int delete_all_messages( int max_resp_time );
int establish_connection();
int network_monitor_disable();
void gsm_apn_configuration(char *apn_name, char *atd_num, char *username, char *password);

#endif /* __GSM_HEADER__ */
