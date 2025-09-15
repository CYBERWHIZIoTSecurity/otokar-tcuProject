#include "include/can_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Global variables for cleanup
static can_bus_t canBus;
static bool running = true;

// Signal handler for graceful shutdown
static void signal_handler(int sig)
{
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = false;
}

// Print usage information
static void print_usage(const char *program_name)
{
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -d <device>    CAN device name (default: can0)\n");
    printf("  -b <bitrate>   Bitrate in bps (default: 500000)\n");
    printf("  -t <timeout>   Receive timeout in ms (default: 1000)\n");
    printf("  -h             Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s                    # Use default settings\n", program_name);
    printf("  %s -d can1            # Use can1 device\n", program_name);
    printf("  %s -b 250000          # Set bitrate to 250 kbps\n", program_name);
    printf("  %s -d can0 -b 1000000 # Use can0 at 1 Mbps\n", program_name);
}

// Send periodic messages
static int send_periodic_messages(can_bus_t *can)
{
    static uint32_t counter = 0;
    uint8_t data[8];
    
    // Prepare data
    data[0] = (counter >> 24) & 0xFF;
    data[1] = (counter >> 16) & 0xFF;
    data[2] = (counter >> 8) & 0xFF;
    data[3] = counter & 0xFF;
    data[4] = 0xAA;
    data[5] = 0xBB;
    data[6] = 0xCC;
    data[7] = 0xDD;
    
    // Send standard frame
    int result = can_bus_send_data(can, 0x123, false, data, 8);
    if (result == CAN_ERROR_NONE) {
        printf("Sent standard frame: ID=0x123, Counter=%u\n", counter);
    } else {
        printf("Failed to send standard frame: %s\n", can_bus_get_error_string(result));
        return result;
    }
    
    // Send extended frame every 10th message
    if (counter % 10 == 0) {
        uint8_t ext_data[4] = {0x11, 0x22, 0x33, 0x44};
        result = can_bus_send_data(can, 0x18FF1234, true, ext_data, 4);
        if (result == CAN_ERROR_NONE) {
            printf("Sent extended frame: ID=0x18FF1234\n");
        } else {
            printf("Failed to send extended frame: %s\n", can_bus_get_error_string(result));
        }
    }
    
    // Send remote frame every 20th message
    if (counter % 20 == 0) {
        result = can_bus_send_remote(can, 0x456, false);
        if (result == CAN_ERROR_NONE) {
            printf("Sent remote frame: ID=0x456\n");
        } else {
            printf("Failed to send remote frame: %s\n", can_bus_get_error_string(result));
        }
    }
    
    counter++;
    return CAN_ERROR_NONE;
}

// Receive and display messages
static int receive_messages(can_bus_t *can, int timeout_ms)
{
    can_frame_t frame;
    int result = can_bus_receive_frame(can, &frame, timeout_ms);
    
    if (result == CAN_ERROR_NONE) {
        printf("Received frame: ");
        can_bus_print_frame(&frame);
    } else if (result == CAN_ERROR_TIMEOUT) {
        // Timeout is normal, don't print error
        return CAN_ERROR_NONE;
    } else {
        printf("Failed to receive frame: %s\n", can_bus_get_error_string(result));
        return result;
    }
    
    return CAN_ERROR_NONE;
}

// Display CAN bus status
static void display_status(can_bus_t *can)
{
    printf("\n=== CAN Bus Status ===\n");
    
    // Get state
    can_bus_state_t state = can_bus_get_state(can);
    printf("State: ");
    switch (state) {
        case CAN_STATE_ERROR_ACTIVE:
            printf("ERROR_ACTIVE\n");
            break;
        case CAN_STATE_ERROR_PASSIVE:
            printf("ERROR_PASSIVE\n");
            break;
        case CAN_STATE_BUS_OFF:
            printf("BUS_OFF\n");
            break;
        case CAN_STATE_STOPPED:
            printf("STOPPED\n");
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }
    
    // Get statistics
    can_statistics_t stats;
    if (can_bus_get_statistics(can, &stats) == CAN_ERROR_NONE) {
        printf("Statistics:\n");
        printf("  TX Frames: %u\n", stats.tx_frames);
        printf("  RX Frames: %u\n", stats.rx_frames);
        printf("  Error Frames: %u\n", stats.error_frames);
        printf("  Bus Errors: %u\n", stats.bus_errors);
    }
    
    // Get error counters
    uint32_t tx_errors, rx_errors;
    if (can_bus_get_error_counters(can, &tx_errors, &rx_errors) == CAN_ERROR_NONE) {
        printf("Error Counters:\n");
        printf("  TX Errors: %u\n", tx_errors);
        printf("  RX Errors: %u\n", rx_errors);
    }
    
    printf("=====================\n\n");
}

// Main function
int main(int argc, char *argv[])
{
    char device_name[64] = "can0";
    uint32_t bitrate = 500000;
    int timeout_ms = 1000;
    int opt;
    
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "d:b:t:h")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(device_name, optarg, sizeof(device_name) - 1);
                device_name[sizeof(device_name) - 1] = '\0';
                break;
            case 'b':
                bitrate = strtoul(optarg, NULL, 10);
                break;
            case 't':
                timeout_ms = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("CAN Bus Example Program\n");
    printf("=======================\n");
    printf("Device: %s\n", device_name);
    printf("Bitrate: %u bps\n", bitrate);
    printf("Timeout: %d ms\n", timeout_ms);
    printf("Press Ctrl+C to stop\n\n");
    
    // Initialize CAN bus
    printf("Initializing CAN bus...\n");
    int result = can_bus_init(&canBus, device_name);
    if (result != CAN_ERROR_NONE) {
        printf("Failed to initialize CAN bus: %s\n", can_bus_get_error_string(result));
        printf("Make sure the CAN device '%s' exists and you have permission to access it.\n", device_name);
        printf("You may need to:\n");
        printf("1. Load the CAN module: sudo modprobe can\n");
        printf("2. Create a virtual CAN interface: sudo ip link add dev %s type vcan\n", device_name);
        printf("3. Bring it up: sudo ip link set up %s\n", device_name);
        return 1;
    }
    
    // Configure CAN bus
    printf("Configuring CAN bus...\n");
    can_config_t config;
    config.bitrate = bitrate;
    config.sample_point = 75;
    config.loopback = false;
    config.listen_only = false;
    config.auto_retransmit = true;
    config.max_retransmissions = 3;
    
    result = can_bus_configure(&canBus, &config);
    if (result != CAN_ERROR_NONE) {
        printf("Failed to configure CAN bus: %s\n", can_bus_get_error_string(result));
        can_bus_deinit(&canBus);
        return 1;
    }
    
    // Start CAN bus
    printf("Starting CAN bus...\n");
    result = can_bus_start(&canBus);
    if (result != CAN_ERROR_NONE) {
        printf("Failed to start CAN bus: %s\n", can_bus_get_error_string(result));
        can_bus_deinit(&canBus);
        return 1;
    }
    
    printf("CAN bus started successfully!\n\n");
    
    // Main loop
    int message_counter = 0;
    while (running) {
        // Send periodic messages every second
        if (message_counter % 10 == 0) { // Every 10 iterations (1 second)
            result = send_periodic_messages(&canBus);
            if (result != CAN_ERROR_NONE) {
                printf("Error sending messages: %s\n", can_bus_get_error_string(result));
            }
        }
        
        // Receive messages
        result = receive_messages(&canBus, 100); // 100ms timeout
        if (result != CAN_ERROR_NONE && result != CAN_ERROR_TIMEOUT) {
            printf("Error receiving messages: %s\n", can_bus_get_error_string(result));
        }
        
        // Display status every 5 seconds
        if (message_counter % 50 == 0) {
            display_status(&canBus);
        }
        
        message_counter++;
        usleep(100000); // 100ms delay
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    can_bus_stop(&canBus);
    can_bus_deinit(&canBus);
    printf("Cleanup complete.\n");
    
    return 0;
}
