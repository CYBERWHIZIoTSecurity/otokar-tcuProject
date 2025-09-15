#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// CAN Bus Configuration
#define CAN_MAX_DATA_LENGTH     8
#define CAN_MAX_FRAME_ID        0x1FFFFFFF
#define CAN_STANDARD_ID_MAX     0x7FF
#define CAN_EXTENDED_ID_MAX     0x1FFFFFFF

// CAN Frame Types
#define CAN_FRAME_TYPE_DATA     0x00
#define CAN_FRAME_TYPE_REMOTE   0x01
#define CAN_FRAME_TYPE_ERROR    0x02
#define CAN_FRAME_TYPE_OVERLOAD 0x03

// CAN Error Codes
#define CAN_ERROR_NONE          0x00
#define CAN_ERROR_BIT           0x01
#define CAN_ERROR_STUFF         0x02
#define CAN_ERROR_FORM          0x03
#define CAN_ERROR_ACK           0x04
#define CAN_ERROR_CRC           0x05

// Error code strings
static const char* error_strings[] = {
	"No error",
	"Bit error",
	"Stuff error", 
	"Form error",
	"ACK error",
	"CRC error",
	"Invalid parameter",
	"Device not found",
	"Permission denied",
	"Device busy",
	"Timeout",
	"Buffer full",
	"Not initialized",
	"Invalid state"
};

// Error codes
#define CAN_ERROR_INVALID_PARAM  -1
#define CAN_ERROR_DEVICE_NOT_FOUND -2
#define CAN_ERROR_PERMISSION_DENIED -3
#define CAN_ERROR_DEVICE_BUSY   -4
#define CAN_ERROR_TIMEOUT       -5
#define CAN_ERROR_BUFFER_FULL   -6
#define CAN_ERROR_NOT_INITIALIZED -7
#define CAN_ERROR_INVALID_STATE -8

// CAN Bus States
typedef enum {
    CAN_STATE_ERROR_ACTIVE = 0,
    CAN_STATE_ERROR_PASSIVE,
    CAN_STATE_BUS_OFF,
    CAN_STATE_STOPPED
} can_bus_state_t;

// CAN Frame Structure
typedef struct {
    uint32_t id;                    // CAN ID (11-bit standard or 29-bit extended)
    bool is_extended;               // Extended frame flag
    bool is_remote;                 // Remote frame flag
    bool is_error;                  // Error frame flag
    uint8_t dlc;                    // Data length code (0-8)
    uint8_t data[CAN_MAX_DATA_LENGTH]; // Data payload
    uint64_t timestamp;             // Timestamp in microseconds
} can_frame_t;

// CAN Message Structure
typedef struct {
    uint32_t id;                    // CAN ID
    bool is_extended;               // Extended frame flag
    uint8_t dlc;                    // Data length code
    uint8_t data[CAN_MAX_DATA_LENGTH]; // Data payload
    uint32_t period_ms;             // Transmission period in milliseconds
    uint64_t last_tx_time;          // Last transmission time
} can_message_t;

// CAN Bus Statistics
typedef struct {
    uint32_t tx_frames;             // Transmitted frames
    uint32_t rx_frames;             // Received frames
    uint32_t error_frames;          // Error frames
    uint32_t bus_errors;            // Bus errors
    uint32_t arbitration_lost;      // Arbitration lost
    uint32_t overrun_errors;        // Overrun errors
} can_statistics_t;

// CAN Bus Configuration
typedef struct {
    uint32_t bitrate;               // Bitrate in bits per second
    uint32_t sample_point;          // Sample point percentage (0-100)
    bool loopback;                  // Loopback mode
    bool listen_only;               // Listen-only mode
    bool auto_retransmit;           // Auto-retransmission
    uint32_t max_retransmissions;   // Maximum retransmissions
} can_config_t;

// CAN Bus Handle
typedef struct {
    int fd;                         // File descriptor
    char device_name[64];           // Device name
    can_config_t config;            // Configuration
    can_bus_state_t state;         // Current bus state
    can_statistics_t stats;         // Statistics
    pthread_mutex_t mutex;          // Mutex for thread safety
    bool is_initialized;            // Initialization flag
} can_bus_t;

// Function Prototypes (camelCase)

// Initialization and Configuration
int can_bus_init(can_bus_t *can, const char *device_name);
int can_bus_deinit(can_bus_t *can);
int can_bus_configure(can_bus_t *can, const can_config_t *config);
int can_bus_start(can_bus_t *can);
int can_bus_stop(can_bus_t *can);
int can_bus_reset(can_bus_t *can);

// Frame Transmission and Reception
int can_bus_send_frame(can_bus_t *can, const can_frame_t *frame);
int can_bus_send_data(can_bus_t *can, uint32_t id, bool is_extended, 
                      const uint8_t *data, uint8_t dlc);
int can_bus_send_remote(can_bus_t *can, uint32_t id, bool is_extended);
int can_bus_receive_frame(can_bus_t *can, can_frame_t *frame, int timeout_ms);

// Message Management
int can_bus_add_message(can_bus_t *can, const can_message_t *message);
int can_bus_remove_message(can_bus_t *can, uint32_t id);
int can_bus_update_message(can_bus_t *can, const can_message_t *message);
int can_bus_process_messages(can_bus_t *can);

// Status and Information
can_bus_state_t can_bus_get_state(can_bus_t *can);
int can_bus_get_statistics(can_bus_t *can, can_statistics_t *stats);
int can_bus_clear_statistics(can_bus_t *can);
int can_bus_get_error_counters(can_bus_t *can, uint32_t *tx_errors, uint32_t *rx_errors);

// Utility Functions
uint32_t can_bus_calculate_bitrate(uint32_t clock_freq, uint32_t prescaler, uint32_t sjw, 
                                   uint32_t bs1, uint32_t bs2);
bool can_bus_is_valid_id(uint32_t id, bool is_extended);
int can_bus_validate_frame(const can_frame_t *frame);
void can_bus_print_frame(const can_frame_t *frame);
void can_bus_print_statistics(const can_statistics_t *stats);

// Error Handling
const char* can_bus_get_error_string(int error_code);
int can_bus_get_last_error(can_bus_t *can);

#ifdef __cplusplus
}
#endif

#endif // CAN_BUS_H
