#include "include/can_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <time.h>

// Internal function prototypes
static int can_bus_set_bitrate(can_bus_t *can, uint32_t bitrate);
static int can_bus_set_mode(can_bus_t *can, const can_config_t *config);
static uint64_t get_timestamp_us(void);

// Initialize CAN bus
int can_bus_init(can_bus_t *can, const char *device_name)
{
    if (!can || !device_name) {
        return CAN_ERROR_INVALID_PARAM;
    }

    // Initialize structure
    memset(can, 0, sizeof(can_bus_t));
    strncpy(can->device_name, device_name, sizeof(can->device_name) - 1);
    can->device_name[sizeof(can->device_name) - 1] = '\0';
    
    // Initialize mutex
    if (pthread_mutex_init(&can->mutex, NULL) != 0) {
        return CAN_ERROR_INVALID_PARAM;
    }

    // Open CAN socket
    can->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can->fd < 0) {
        pthread_mutex_destroy(&can->mutex);
        return CAN_ERROR_DEVICE_NOT_FOUND;
    }

    // Get interface index
    struct ifreq ifr;
    strcpy(ifr.ifr_name, device_name);
    if (ioctl(can->fd, SIOCGIFINDEX, &ifr) < 0) {
        close(can->fd);
        pthread_mutex_destroy(&can->mutex);
        return CAN_ERROR_DEVICE_NOT_FOUND;
    }

    // Bind socket to interface
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(can->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(can->fd);
        pthread_mutex_destroy(&can->mutex);
        return CAN_ERROR_PERMISSION_DENIED;
    }

    // Set default configuration
    can->config.bitrate = 500000;  // 500 kbps default
    can->config.sample_point = 75; // 75% sample point
    can->config.loopback = false;
    can->config.listen_only = false;
    can->config.auto_retransmit = true;
    can->config.max_retransmissions = 3;

    can->state = CAN_STATE_ERROR_ACTIVE;
    can->is_initialized = true;

    return CAN_ERROR_NONE;
}

// Deinitialize CAN bus
int can_bus_deinit(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    
    if (can->fd >= 0) {
        close(can->fd);
        can->fd = -1;
    }
    
    can->is_initialized = false;
    can->state = CAN_STATE_STOPPED;
    
    pthread_mutex_unlock(&can->mutex);
    pthread_mutex_destroy(&can->mutex);
    
    return CAN_ERROR_NONE;
}

// Configure CAN bus
int can_bus_configure(can_bus_t *can, const can_config_t *config)
{
    if (!can || !can->is_initialized || !config) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Copy configuration
    memcpy(&can->config, config, sizeof(can_config_t));
    
    // Apply configuration
    int result = can_bus_set_bitrate(can, config->bitrate);
    if (result == CAN_ERROR_NONE) {
        result = can_bus_set_mode(can, config);
    }
    
    pthread_mutex_unlock(&can->mutex);
    return result;
}

// Start CAN bus
int can_bus_start(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Set interface up
    struct ifreq ifr;
    strcpy(ifr.ifr_name, can->device_name);
    if (ioctl(can->fd, SIOCGIFFLAGS, &ifr) < 0) {
        pthread_mutex_unlock(&can->mutex);
        return CAN_ERROR_DEVICE_NOT_FOUND;
    }
    
    ifr.ifr_flags |= IFF_UP;
    if (ioctl(can->fd, SIOCSIFFLAGS, &ifr) < 0) {
        pthread_mutex_unlock(&can->mutex);
        return CAN_ERROR_PERMISSION_DENIED;
    }
    
    can->state = CAN_STATE_ERROR_ACTIVE;
    
    pthread_mutex_unlock(&can->mutex);
    return CAN_ERROR_NONE;
}

// Stop CAN bus
int can_bus_stop(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Set interface down
    struct ifreq ifr;
    strcpy(ifr.ifr_name, can->device_name);
    if (ioctl(can->fd, SIOCGIFFLAGS, &ifr) < 0) {
        pthread_mutex_unlock(&can->mutex);
        return CAN_ERROR_DEVICE_NOT_FOUND;
    }
    
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(can->fd, SIOCSIFFLAGS, &ifr) < 0) {
        pthread_mutex_unlock(&can->mutex);
        return CAN_ERROR_PERMISSION_DENIED;
    }
    
    can->state = CAN_STATE_STOPPED;
    
    pthread_mutex_unlock(&can->mutex);
    return CAN_ERROR_NONE;
}

// Reset CAN bus
int can_bus_reset(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Clear statistics
    memset(&can->stats, 0, sizeof(can_statistics_t));
    
    // Reset state
    can->state = CAN_STATE_ERROR_ACTIVE;
    
    pthread_mutex_unlock(&can->mutex);
    return CAN_ERROR_NONE;
}

// Send CAN frame
int can_bus_send_frame(can_bus_t *can, const can_frame_t *frame)
{
    if (!can || !can->is_initialized || !frame) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    if (can->state != CAN_STATE_ERROR_ACTIVE) {
        return CAN_ERROR_INVALID_STATE;
    }

    // Validate frame
    int validation = can_bus_validate_frame(frame);
    if (validation != CAN_ERROR_NONE) {
        return validation;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Prepare Linux CAN frame
    struct can_frame linux_frame;
    memset(&linux_frame, 0, sizeof(linux_frame));
    
    if (frame->is_extended) {
        linux_frame.can_id = frame->id | CAN_EFF_FLAG;
    } else {
        linux_frame.can_id = frame->id & CAN_SFF_MASK;
    }
    
    if (frame->is_remote) {
        linux_frame.can_id |= CAN_RTR_FLAG;
    }
    
    linux_frame.can_dlc = frame->dlc;
    if (!frame->is_remote) {
        memcpy(linux_frame.data, frame->data, frame->dlc);
    }
    
    // Send frame
    ssize_t bytes_sent = write(can->fd, &linux_frame, sizeof(linux_frame));
    if (bytes_sent < 0) {
        pthread_mutex_unlock(&can->mutex);
        can->stats.bus_errors++;
        return CAN_ERROR_DEVICE_BUSY;
    }
    
    // Update statistics
    can->stats.tx_frames++;
    
    pthread_mutex_unlock(&can->mutex);
    return CAN_ERROR_NONE;
}

// Send data frame
int can_bus_send_data(can_bus_t *can, uint32_t id, bool is_extended, 
                      const uint8_t *data, uint8_t dlc)
{
    if (!can || !can->is_initialized || !data) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    can_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    
    frame.id = id;
    frame.is_extended = is_extended;
    frame.is_remote = false;
    frame.is_error = false;
    frame.dlc = dlc;
    frame.timestamp = get_timestamp_us();
    
    memcpy(frame.data, data, dlc);
    
    return can_bus_send_frame(can, &frame);
}

// Send remote frame
int can_bus_send_remote(can_bus_t *can, uint32_t id, bool is_extended)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    can_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    
    frame.id = id;
    frame.is_extended = is_extended;
    frame.is_remote = true;
    frame.is_error = false;
    frame.dlc = 0;
    frame.timestamp = get_timestamp_us();
    
    return can_bus_send_frame(can, &frame);
}

// Receive CAN frame
int can_bus_receive_frame(can_bus_t *can, can_frame_t *frame, int timeout_ms)
{
    if (!can || !can->is_initialized || !frame) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    if (can->state == CAN_STATE_STOPPED) {
        return CAN_ERROR_INVALID_STATE;
    }

    pthread_mutex_lock(&can->mutex);
    
    // Set timeout if specified
    if (timeout_ms > 0) {
        fd_set read_fds;
        struct timeval timeout;
        
        FD_ZERO(&read_fds);
        FD_SET(can->fd, &read_fds);
        
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        
        int select_result = select(can->fd + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result <= 0) {
            pthread_mutex_unlock(&can->mutex);
            return CAN_ERROR_TIMEOUT;
        }
    }
    
    // Receive frame
    struct can_frame linux_frame;
    ssize_t bytes_received = read(can->fd, &linux_frame, sizeof(linux_frame));
    if (bytes_received < 0) {
        pthread_mutex_unlock(&can->mutex);
        return CAN_ERROR_DEVICE_BUSY;
    }
    
    // Convert to our frame format
    memset(frame, 0, sizeof(can_frame_t));
    
    if (linux_frame.can_id & CAN_EFF_FLAG) {
        frame->id = linux_frame.can_id & CAN_EFF_MASK;
        frame->is_extended = true;
    } else {
        frame->id = linux_frame.can_id & CAN_SFF_MASK;
        frame->is_extended = false;
    }
    
    frame->is_remote = (linux_frame.can_id & CAN_RTR_FLAG) ? true : false;
    frame->is_error = (linux_frame.can_id & CAN_ERR_FLAG) ? true : false;
    frame->dlc = linux_frame.can_dlc;
    frame->timestamp = get_timestamp_us();
    
    if (!frame->is_remote && !frame->is_error) {
        memcpy(frame->data, linux_frame.data, frame->dlc);
    }
    
    // Update statistics
    if (frame->is_error) {
        can->stats.error_frames++;
    } else {
        can->stats.rx_frames++;
    }
    
    pthread_mutex_unlock(&can->mutex);
    return CAN_ERROR_NONE;
}

// Get CAN bus state
can_bus_state_t can_bus_get_state(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_STATE_STOPPED;
    }
    
    return can->state;
}

// Get statistics
int can_bus_get_statistics(can_bus_t *can, can_statistics_t *stats)
{
    if (!can || !can->is_initialized || !stats) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    memcpy(stats, &can->stats, sizeof(can_statistics_t));
    pthread_mutex_unlock(&can->mutex);
    
    return CAN_ERROR_NONE;
}

// Clear statistics
int can_bus_clear_statistics(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    pthread_mutex_lock(&can->mutex);
    memset(&can->stats, 0, sizeof(can_statistics_t));
    pthread_mutex_unlock(&can->mutex);
    
    return CAN_ERROR_NONE;
}

// Get error counters
int can_bus_get_error_counters(can_bus_t *can, uint32_t *tx_errors, uint32_t *rx_errors)
{
    if (!can || !can->is_initialized || !tx_errors || !rx_errors) {
        return CAN_ERROR_NOT_INITIALIZED;
    }

    // This would require additional ioctl calls to get actual error counters
    // For now, return statistics
    pthread_mutex_lock(&can->mutex);
    *tx_errors = can->stats.bus_errors;
    *rx_errors = can->stats.error_frames;
    pthread_mutex_unlock(&can->mutex);
    
    return CAN_ERROR_NONE;
}

// Calculate bitrate
uint32_t can_bus_calculate_bitrate(uint32_t clock_freq, uint32_t prescaler, uint32_t sjw, 
                                   uint32_t bs1, uint32_t bs2)
{
    if (prescaler == 0 || bs1 == 0 || bs2 == 0) {
        return 0;
    }
    
    uint32_t total_quanta = 1 + bs1 + bs2;
    return clock_freq / (prescaler * total_quanta);
}

// Check if CAN ID is valid
bool can_bus_is_valid_id(uint32_t id, bool is_extended)
{
    if (is_extended) {
        return id <= CAN_EXTENDED_ID_MAX;
    } else {
        return id <= CAN_STANDARD_ID_MAX;
    }
}

// Validate CAN frame
int can_bus_validate_frame(const can_frame_t *frame)
{
    if (!frame) {
        return CAN_ERROR_INVALID_PARAM;
    }
    
    if (!can_bus_is_valid_id(frame->id, frame->is_extended)) {
        return CAN_ERROR_INVALID_PARAM;
    }
    
    if (frame->dlc > CAN_MAX_DATA_LENGTH) {
        return CAN_ERROR_INVALID_PARAM;
    }
    
    if (frame->is_remote && frame->dlc != 0) {
        return CAN_ERROR_INVALID_PARAM;
    }
    
    return CAN_ERROR_NONE;
}

// Print CAN frame
void can_bus_print_frame(const can_frame_t *frame)
{
    if (!frame) {
        printf("Invalid frame pointer\n");
        return;
    }
    
    printf("CAN Frame: ID=0x%03X%s, DLC=%d, %s, %s\n",
           frame->id,
           frame->is_extended ? "X" : "",
           frame->dlc,
           frame->is_remote ? "REMOTE" : "DATA",
           frame->is_error ? "ERROR" : "NORMAL");
    
    if (!frame->is_remote && !frame->is_error && frame->dlc > 0) {
        printf("Data: ");
        for (int i = 0; i < frame->dlc; i++) {
            printf("%02X ", frame->data[i]);
        }
        printf("\n");
    }
    
    printf("Timestamp: %llu us\n", (unsigned long long)frame->timestamp);
}

// Print statistics
void can_bus_print_statistics(const can_statistics_t *stats)
{
    if (!stats) {
        printf("Invalid statistics pointer\n");
        return;
    }
    
    printf("CAN Bus Statistics:\n");
    printf("  TX Frames: %u\n", stats->tx_frames);
    printf("  RX Frames: %u\n", stats->rx_frames);
    printf("  Error Frames: %u\n", stats->error_frames);
    printf("  Bus Errors: %u\n", stats->bus_errors);
    printf("  Arbitration Lost: %u\n", stats->arbitration_lost);
    printf("  Overrun Errors: %u\n", stats->overrun_errors);
}

// Get error string
const char* can_bus_get_error_string(int error_code)
{
    if (error_code >= 0) {
        return "No error";
    }
    
    int index = -error_code;
    if (index < (int)(sizeof(error_strings) / sizeof(error_strings[0]))) {
        return error_strings[index];
    }
    
    return "Unknown error";
}

// Get last error (placeholder - would need to implement error tracking)
int can_bus_get_last_error(can_bus_t *can)
{
    if (!can || !can->is_initialized) {
        return CAN_ERROR_NOT_INITIALIZED;
    }
    
    // This would require implementing error tracking
    return CAN_ERROR_NONE;
}

// Set bitrate (internal function)
static int can_bus_set_bitrate(can_bus_t *can, uint32_t bitrate)
{
    // This would require ioctl calls to set the bitrate
    // For now, just store the value
    can->config.bitrate = bitrate;
    return CAN_ERROR_NONE;
}

// Set mode (internal function)
static int can_bus_set_mode(can_bus_t *can, const can_config_t *config)
{
    // This would require ioctl calls to set various modes
    // For now, just store the values
    can->config.loopback = config->loopback;
    can->config.listen_only = config->listen_only;
    return CAN_ERROR_NONE;
}

// Get timestamp in microseconds (internal function)
static uint64_t get_timestamp_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

// Placeholder functions for message management (would need additional implementation)
int can_bus_add_message(can_bus_t *can, const can_message_t *message)
{
    // This would require implementing a message queue
    return CAN_ERROR_NONE;
}

int can_bus_remove_message(can_bus_t *can, uint32_t id)
{
    // This would require implementing a message queue
    return CAN_ERROR_NONE;
}

int can_bus_update_message(can_bus_t *can, const can_message_t *message)
{
    // This would require implementing a message queue
    return CAN_ERROR_NONE;
}

int can_bus_process_messages(can_bus_t *can)
{
    // This would require implementing a message queue
    return CAN_ERROR_NONE;
}
