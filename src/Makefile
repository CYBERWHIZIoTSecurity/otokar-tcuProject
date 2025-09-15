SRC_DIR := .
INC_DIR := ./include

SRCS := canbus-interface.c

OBJ_DIR := ../build/obj
BIN_DIR := ../build/bin

OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

CC ?= aarch64-poky-linux-gcc
CFLAGS  += -I$(INC_DIR) -Wall -O2 -g -std=c99

LIB_DIR := ../Telematics_GW_library/lib
LDFLAGS += -lpthread -lm -L$(LIB_DIR) -lTelematics_GW

APP_NAME := tcu-app

TARGET := $(BIN_DIR)/$(APP_NAME)

tcu-app: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf ../build

help:
	@echo "Available targets:"
	@echo "  tcu-app   - Build TCU application"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help message"

.PHONY: tcu-app clean help