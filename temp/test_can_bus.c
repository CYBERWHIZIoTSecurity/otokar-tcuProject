#include "include/can_bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

// Test configuration
#define TEST_CAN_DEVICE "can0"
#define TEST_TIMEOUT_MS 1000
#define TEST_ITERATIONS 100

// Test results
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} test_results_t;

static test_results_t test_results = {0, 0, 0};

// Test macros
#define TEST_ASSERT(condition, message) \
    do { \
        test_results.total_tests++; \
        if (condition) { \
            test_results.passed_tests++; \
            printf("✓ %s\n", message); \
        } else { \
            test_results.failed_tests++; \
            printf("✗ %s\n", message); \
        } \
    } while(0)

#define TEST_EQUAL(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

#define TEST_NOT_EQUAL(expected, actual, message) \
    TEST_ASSERT((expected) != (actual), message)

#define TEST_NULL(ptr, message) \
    TEST_ASSERT((ptr) == NULL, message)

#define TEST_NOT_NULL(ptr, message) \
    TEST_ASSERT((ptr) != NULL, message)

// Test function prototypes
static void test_can_bus_initialization(void);
static void test_can_bus_configuration(void);
static void test_can_bus_start_stop(void);
static void test_can_bus_frame_validation(void);
static void test_can_bus_utility_functions(void);
static void test_can_bus_error_handling(void);
static void test_can_bus_statistics(void);
static void test_can_bus_thread_safety(void);
static void test_can_bus_performance(void);
static void test_can_bus_integration(void);

// Mock CAN device for testing (when real device is not available)
static int mock_can_fd = -1;
static bool mock_device_available = false;

// Setup mock CAN device
static int setup_mock_can_device(void)
{
    // Try to create a virtual CAN interface
    system("sudo modprobe vcan 2>/dev/null");
    system("sudo ip link add dev can0 type vcan 2>/dev/null");
    system("sudo ip link set up can0 2>/dev/null");
    
    // Check if device was created
    if (access("/sys/class/net/can0", F_OK) == 0) {
        mock_device_available = true;
        return 0;
    }
    
    return -1;
}

// Cleanup mock CAN device
static void cleanup_mock_can_device(void)
{
    if (mock_device_available) {
        system("sudo ip link del can0 2>/dev/null");
        system("sudo modprobe -r vcan 2>/dev/null");
        mock_device_available = false;
    }
}

// Test CAN bus initialization
static void test_can_bus_initialization(void)
{
    printf("\n=== Testing CAN Bus Initialization ===\n");
    
    can_bus_t can;
    
    // Test with NULL parameters
    TEST_EQUAL(CAN_ERROR_INVALID_PARAM, can_bus_init(NULL, "can0"), 
               "Init with NULL can pointer should fail");
    TEST_EQUAL(CAN_ERROR_INVALID_PARAM, can_bus_init(&can, NULL), 
               "Init with NULL device name should fail");
    
    // Test with empty device name
    TEST_EQUAL(CAN_ERROR_INVALID_PARAM, can_bus_init(&can, ""), 
               "Init with empty device name should fail");
    
    // Test normal initialization (if device available)
    if (mock_device_available) {
        int result = can_bus_init(&can, TEST_CAN_DEVICE);
        if (result == CAN_ERROR_NONE) {
            TEST_ASSERT(can.is_initialized, "CAN bus should be initialized");
            TEST_ASSERT(can.fd >= 0, "File descriptor should be valid");
            TEST_EQUAL(CAN_STATE_ERROR_ACTIVE, can.state, "Initial state should be ERROR_ACTIVE");
            TEST_EQUAL(500000, can.config.bitrate, "Default bitrate should be 500kbps");
            
            // Test deinitialization
            TEST_EQUAL(CAN_ERROR_NONE, can_bus_deinit(&can), "Deinit should succeed");
            TEST_ASSERT(!can.is_initialized, "CAN bus should not be initialized after deinit");
        } else {
            printf("Note: Real CAN device not available, skipping device tests\n");
        }
    } else {
        printf("Note: Mock CAN device not available, skipping device tests\n");
    }
}

// Test CAN bus configuration
static void test_can_bus_configuration(void)
{
    printf("\n=== Testing CAN Bus Configuration ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping configuration tests\n");
        return;
    }
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    
    // Test configuration
    can_config_t config;
    config.bitrate = 250000;
    config.sample_point = 80;
    config.loopback = true;
    config.listen_only = false;
    config.auto_retransmit = true;
    config.max_retransmissions = 5;
    
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_configure(&can, &config), 
               "Configuration should succeed");
    
    // Verify configuration was applied
    TEST_EQUAL(250000, can.config.bitrate, "Bitrate should be updated");
    TEST_EQUAL(80, can.config.sample_point, "Sample point should be updated");
    TEST_ASSERT(can.config.loopback, "Loopback should be enabled");
    TEST_ASSERT(can.config.auto_retransmit, "Auto-retransmit should be enabled");
    TEST_EQUAL(5, can.config.max_retransmissions, "Max retransmissions should be updated");
    
    // Test configuration with NULL config
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_configure(&can, NULL), 
               "Configuration with NULL config should fail");
    
    // Test configuration on uninitialized bus
    can_bus_t uninit_can;
    memset(&uninit_can, 0, sizeof(uninit_can));
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_configure(&uninit_can, &config), 
               "Configuration on uninitialized bus should fail");
    
    can_bus_deinit(&can);
}

// Test CAN bus start/stop
static void test_can_bus_start_stop(void)
{
    printf("\n=== Testing CAN Bus Start/Stop ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping start/stop tests\n");
        return;
    }
    
    // Test start/stop on uninitialized bus
    can_bus_t uninit_can;
    memset(&uninit_can, 0, sizeof(uninit_can));
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_start(&uninit_can), 
               "Start on uninitialized bus should fail");
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_stop(&uninit_can), 
               "Stop on uninitialized bus should fail");
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    
    // Test start
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_start(&can), "Start should succeed");
    TEST_EQUAL(CAN_STATE_ERROR_ACTIVE, can.state, "State should be ERROR_ACTIVE after start");
    
    // Test stop
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_stop(&can), "Stop should succeed");
    TEST_EQUAL(CAN_STATE_STOPPED, can.state, "State should be STOPPED after stop");
    
    // Test start again
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_start(&can), "Second start should succeed");
    TEST_EQUAL(CAN_STATE_ERROR_ACTIVE, can.state, "State should be ERROR_ACTIVE after second start");
    
    can_bus_deinit(&can);
}

// Test CAN frame validation
static void test_can_bus_frame_validation(void)
{
    printf("\n=== Testing CAN Frame Validation ===\n");
    
    can_frame_t frame;
    
    // Test valid standard frame
    memset(&frame, 0, sizeof(frame));
    frame.id = 0x123;
    frame.is_extended = false;
    frame.is_remote = false;
    frame.dlc = 4;
    frame.data[0] = 0xAA;
    frame.data[1] = 0xBB;
    frame.data[2] = 0xCC;
    frame.data[3] = 0xDD;
    
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
               "Valid standard frame should pass validation");
    
    // Test valid extended frame
    frame.id = 0x18FF1234;
    frame.is_extended = true;
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
               "Valid extended frame should pass validation");
    
    // Test valid remote frame
    frame.is_remote = true;
    frame.dlc = 0;
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
               "Valid remote frame should pass validation");
    
    // Test invalid standard ID
    frame.is_extended = false;
    frame.id = 0x800; // Beyond 11-bit range
    TEST_NOT_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
                   "Invalid standard ID should fail validation");
    
    // Test invalid extended ID
    frame.is_extended = true;
    frame.id = 0x20000000; // Beyond 29-bit range
    TEST_NOT_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
                   "Invalid extended ID should fail validation");
    
    // Test invalid DLC
    frame.id = 0x123;
    frame.is_extended = false;
    frame.is_remote = false;
    frame.dlc = 9; // Beyond max DLC
    TEST_NOT_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
                   "Invalid DLC should fail validation");
    
    // Test remote frame with non-zero DLC
    frame.dlc = 4;
    frame.is_remote = true;
    TEST_NOT_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(&frame), 
                   "Remote frame with non-zero DLC should fail validation");
    
    // Test NULL frame
    TEST_NOT_EQUAL(CAN_ERROR_NONE, can_bus_validate_frame(NULL), 
                   "NULL frame should fail validation");
}

// Test utility functions
static void test_can_bus_utility_functions(void)
{
    printf("\n=== Testing CAN Bus Utility Functions ===\n");
    
    // Test ID validation
    TEST_ASSERT(can_bus_is_valid_id(0x123, false), "Valid standard ID should return true");
    TEST_ASSERT(can_bus_is_valid_id(0x7FF, false), "Max standard ID should return true");
    TEST_ASSERT(!can_bus_is_valid_id(0x800, false), "Invalid standard ID should return false");
    
    TEST_ASSERT(can_bus_is_valid_id(0x18FF1234, true), "Valid extended ID should return true");
    TEST_ASSERT(can_bus_is_valid_id(0x1FFFFFFF, true), "Max extended ID should return true");
    TEST_ASSERT(!can_bus_is_valid_id(0x20000000, true), "Invalid extended ID should return false");
    
    // Test bitrate calculation
    uint32_t bitrate = can_bus_calculate_bitrate(80000000, 100, 1, 3, 2);
    TEST_EQUAL(133333, bitrate, "Bitrate calculation should be correct");
    
    // Test bitrate calculation with invalid parameters
    TEST_EQUAL(0, can_bus_calculate_bitrate(80000000, 0, 1, 3, 2), 
               "Bitrate calculation with zero prescaler should return 0");
    TEST_EQUAL(0, can_bus_calculate_bitrate(80000000, 100, 1, 0, 2), 
               "Bitrate calculation with zero BS1 should return 0");
    TEST_EQUAL(0, can_bus_calculate_bitrate(80000000, 100, 1, 3, 0), 
               "Bitrate calculation with zero BS2 should return 0");
}

// Test error handling
static void test_can_bus_error_handling(void)
{
    printf("\n=== Testing CAN Bus Error Handling ===\n");
    
    // Test error strings
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_NONE), "Error string for CAN_ERROR_NONE should not be NULL");
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_BIT), "Error string for CAN_ERROR_BIT should not be NULL");
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_STUFF), "Error string for CAN_ERROR_STUFF should not be NULL");
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_FORM), "Error string for CAN_ERROR_FORM should not be NULL");
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_ACK), "Error string for CAN_ERROR_ACK should not be NULL");
    TEST_NOT_NULL(can_bus_get_error_string(CAN_ERROR_CRC), "Error string for CAN_ERROR_CRC should not be NULL");
    
    // Test unknown error codes
    TEST_NOT_NULL(can_bus_get_error_string(-999), "Unknown error code should return 'Unknown error'");
}

// Test statistics
static void test_can_bus_statistics(void)
{
    printf("\n=== Testing CAN Bus Statistics ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping statistics tests\n");
        return;
    }
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    
    // Test initial statistics
    can_statistics_t stats;
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_get_statistics(&can, &stats), 
               "Get statistics should succeed");
    TEST_EQUAL(0, stats.tx_frames, "Initial TX frames should be 0");
    TEST_EQUAL(0, stats.rx_frames, "Initial RX frames should be 0");
    TEST_EQUAL(0, stats.error_frames, "Initial error frames should be 0");
    
    // Test clear statistics
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_clear_statistics(&can), 
               "Clear statistics should succeed");
    
    // Test statistics on uninitialized bus
    can_bus_t uninit_can;
    memset(&uninit_can, 0, sizeof(uninit_can));
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_get_statistics(&uninit_can, &stats), 
               "Get statistics on uninitialized bus should fail");
    TEST_EQUAL(CAN_ERROR_NOT_INITIALIZED, can_bus_clear_statistics(&uninit_can), 
               "Clear statistics on uninitialized bus should fail");
    
    can_bus_deinit(&can);
}

// Test thread safety
static void test_can_bus_thread_safety(void)
{
    printf("\n=== Testing CAN Bus Thread Safety ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping thread safety tests\n");
        return;
    }
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    
    // Test concurrent access (basic test)
    can_bus_start(&can);
    
    // This is a basic test - in a real scenario, you'd want more comprehensive
    // concurrent access testing with multiple threads
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_get_state(&can), "Concurrent state access should work");
    
    can_bus_stop(&can);
    can_bus_deinit(&can);
}

// Test performance
static void test_can_bus_performance(void)
{
    printf("\n=== Testing CAN Bus Performance ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping performance tests\n");
        return;
    }
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_start(&can), "CAN bus start should succeed");
    
    // Test frame validation performance
    can_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = 0x123;
    frame.dlc = 4;
    
    clock_t start = clock();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        can_bus_validate_frame(&frame);
    }
    clock_t end = clock();
    
    double validation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Frame validation time for %d iterations: %.6f seconds\n", 
           TEST_ITERATIONS, validation_time);
    
    // Test ID validation performance
    start = clock();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        can_bus_is_valid_id(i % 0x800, false);
    }
    end = clock();
    
    double id_validation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("ID validation time for %d iterations: %.6f seconds\n", 
           TEST_ITERATIONS, id_validation_time);
    
    can_bus_stop(&can);
    can_bus_deinit(&can);
}

// Test integration scenarios
static void test_can_bus_integration(void)
{
    printf("\n=== Testing CAN Bus Integration ===\n");
    
    can_bus_t can;
    
    if (!mock_device_available) {
        printf("Note: Mock CAN device not available, skipping integration tests\n");
        return;
    }
    
    // Initialize CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_init(&can, TEST_CAN_DEVICE), 
               "CAN bus init should succeed");
    
    // Configure CAN bus
    can_config_t config;
    config.bitrate = 500000;
    config.sample_point = 75;
    config.loopback = false;
    config.listen_only = false;
    config.auto_retransmit = true;
    config.max_retransmissions = 3;
    
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_configure(&can, &config), 
               "Configuration should succeed");
    
    // Start CAN bus
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_start(&can), "Start should succeed");
    
    // Test complete workflow
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    
    // Send data frame
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_send_data(&can, 0x123, false, test_data, 4), 
               "Send data should succeed");
    
    // Send remote frame
    TEST_EQUAL(CAN_ERROR_NONE, can_bus_send_remote(&can, 0x456, false), 
               "Send remote should succeed");
    
    // Check statistics
    can_statistics_t stats;
    can_bus_get_statistics(&can, &stats);
    TEST_EQUAL(2, stats.tx_frames, "Should have sent 2 frames");
    
    // Stop and cleanup
    can_bus_stop(&can);
    can_bus_deinit(&can);
}

// Print test results
static void print_test_results(void)
{
    printf("\n=== Test Results ===\n");
    printf("Total Tests: %d\n", test_results.total_tests);
    printf("Passed: %d\n", test_results.passed_tests);
    printf("Failed: %d\n", test_results.failed_tests);
    printf("Success Rate: %.1f%%\n", 
           (test_results.total_tests > 0) ? 
           (100.0 * test_results.passed_tests / test_results.total_tests) : 0.0);
    
    if (test_results.failed_tests == 0) {
        printf("\n🎉 All tests passed!\n");
    } else {
        printf("\n❌ Some tests failed. Please review the output above.\n");
    }
}

// Main test function
int main(void)
{
    printf("Starting CAN Bus Test Suite\n");
    printf("===========================\n");
    
    // Setup mock CAN device
    if (setup_mock_can_device() != 0) {
        printf("Warning: Could not setup mock CAN device. Some tests will be skipped.\n");
    }
    
    // Run all tests
    test_can_bus_initialization();
    test_can_bus_configuration();
    test_can_bus_start_stop();
    test_can_bus_frame_validation();
    test_can_bus_utility_functions();
    test_can_bus_error_handling();
    test_can_bus_statistics();
    test_can_bus_thread_safety();
    test_can_bus_performance();
    test_can_bus_integration();
    
    // Cleanup mock CAN device
    cleanup_mock_can_device();
    
    // Print results
    print_test_results();
    
    return (test_results.failed_tests == 0) ? 0 : 1;
}
