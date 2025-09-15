#!/bin/bash

# CAN Bus Implementation Build Script
# This script provides easy commands for building, testing, and running the project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project directories
SRC_DIR="src"
BUILD_DIR="build"
BIN_DIR="$BUILD_DIR/bin"

# Default compiler
CC="${CC:-aarch64-poky-linux-gcc}"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check prerequisites
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Check if make is available
    if ! command -v make &> /dev/null; then
        print_error "make is not installed. Please install build-essential package."
        exit 1
    fi
    
    # Check if compiler is available
    if ! command -v $CC &> /dev/null; then
        print_warning "Cross-compiler $CC not found. Trying native gcc..."
        if command -v gcc &> /dev/null; then
            CC=gcc
            print_status "Using native gcc compiler"
        else
            print_error "No suitable compiler found. Please install gcc or set CC environment variable."
            exit 1
        fi
    fi
    
    print_success "Prerequisites check passed"
}

# Function to setup virtual CAN interface for testing
setup_vcan() {
    print_status "Setting up virtual CAN interface for testing..."
    
    # Check if vcan module is loaded
    if ! lsmod | grep -q vcan; then
        print_status "Loading vcan module..."
        sudo modprobe vcan 2>/dev/null || print_warning "Could not load vcan module (may need sudo)"
    fi
    
    # Check if can0 interface exists
    if ! ip link show can0 &>/dev/null; then
        print_status "Creating virtual CAN interface can0..."
        sudo ip link add dev can0 type vcan 2>/dev/null || print_warning "Could not create can0 interface (may need sudo)"
    fi
    
    # Bring interface up
    if ip link show can0 &>/dev/null; then
        print_status "Bringing can0 interface up..."
        sudo ip link set up can0 2>/dev/null || print_warning "Could not bring can0 up (may need sudo)"
    fi
    
    print_success "Virtual CAN interface setup complete"
}

# Function to cleanup virtual CAN interface
cleanup_vcan() {
    print_status "Cleaning up virtual CAN interface..."
    
    if ip link show can0 &>/dev/null; then
        sudo ip link del can0 2>/dev/null || print_warning "Could not delete can0 interface"
    fi
    
    print_success "Cleanup complete"
}

# Function to build the project
build_project() {
    print_status "Building CAN Bus implementation..."
    
    # Create build directory
    mkdir -p $BUILD_DIR
    
    # Build with specified compiler
    cd $SRC_DIR
    make CC="$CC" clean
    make CC="$CC" all
    
    if [ $? -eq 0 ]; then
        print_success "Build completed successfully"
        print_status "Binaries created in $BIN_DIR/"
        ls -la $BIN_DIR/
    else
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
}

# Function to run tests
run_tests() {
    print_status "Running CAN Bus test suite..."
    
    if [ ! -f "$BIN_DIR/can-bus-test" ]; then
        print_error "Test binary not found. Please build the project first."
        exit 1
    fi
    
    # Setup virtual CAN interface for testing
    setup_vcan
    
    # Run tests
    print_status "Executing tests..."
    $BIN_DIR/can-bus-test
    
    if [ $? -eq 0 ]; then
        print_success "All tests passed!"
    else
        print_error "Some tests failed"
        exit 1
    fi
    
    # Cleanup virtual CAN interface
    cleanup_vcan
}

# Function to run example program
run_example() {
    print_status "Running CAN Bus example program..."
    
    if [ ! -f "$BIN_DIR/can-example" ]; then
        print_error "Example binary not found. Please build the project first."
        exit 1
    fi
    
    # Setup virtual CAN interface
    setup_vcan
    
    print_status "Starting example program (press Ctrl+C to stop)..."
    print_status "The program will send periodic CAN messages and display statistics"
    
    # Run example with trap for cleanup
    trap cleanup_vcan EXIT
    $BIN_DIR/can-example
    
    print_success "Example program completed"
}

# Function to show help
show_help() {
    echo "CAN Bus Implementation Build Script"
    echo "=================================="
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build     Build the entire project"
    echo "  test      Build and run the test suite"
    echo "  example   Build and run the example program"
    echo "  clean     Clean build artifacts"
    echo "  setup     Setup virtual CAN interface for testing"
    echo "  cleanup   Cleanup virtual CAN interface"
    echo "  help      Show this help message"
    echo ""
    echo "Environment Variables:"
    echo "  CC        Compiler to use (default: aarch64-poky-linux-gcc)"
    echo ""
    echo "Examples:"
    echo "  $0 build          # Build the project"
    echo "  $0 test           # Run tests"
    echo "  $0 example        # Run example program"
    echo "  CC=gcc $0 build   # Build with native gcc"
    echo ""
}

# Function to clean build artifacts
clean_build() {
    print_status "Cleaning build artifacts..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf $BUILD_DIR
        print_success "Build directory removed"
    else
        print_status "No build directory to clean"
    fi
    
    if [ -d "$SRC_DIR" ]; then
        cd $SRC_DIR
        make clean 2>/dev/null || true
        cd ..
        print_success "Source directory cleaned"
    fi
}

# Main script logic
main() {
    case "${1:-help}" in
        "build")
            check_prerequisites
            build_project
            ;;
        "test")
            check_prerequisites
            build_project
            run_tests
            ;;
        "example")
            check_prerequisites
            build_project
            run_example
            ;;
        "clean")
            clean_build
            ;;
        "setup")
            setup_vcan
            ;;
        "cleanup")
            cleanup_vcan
            ;;
        "help"|*)
            show_help
            ;;
    esac
}

# Check if script is being sourced or executed
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    # Script is being executed
    main "$@"
else
    # Script is being sourced
    print_status "CAN Bus build script loaded. Use 'build.sh <command>' to execute commands."
fi
