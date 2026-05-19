CC       := gcc
CFLAGS   := -Wall -Wextra -g -Iinclude -Ilib
LDFLAGS  :=

SRC_DIR  := src
LIB_DIR  := lib
TEST_DIR := tests
BUILD_DIR:= build

LIB_SRCS := $(LIB_DIR)/gluethread/glthread.c
SRCS     := $(SRC_DIR)/mm.c
TEST_SRC := $(TEST_DIR)/testapp.c
TARGET   := $(BUILD_DIR)/testapp

OBJS     := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS) $(LIB_SRCS) $(TEST_SRC)))

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR)/mm.o: $(SRC_DIR)/mm.c | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/glthread.o: $(LIB_DIR)/gluethread/glthread.c | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/testapp.o: $(TEST_DIR)/testapp.c | $(BUILD_DIR)
    $(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(BUILD_DIR)/mm.o $(BUILD_DIR)/glthread.o $(BUILD_DIR)/testapp.o
    $(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
    mkdir -p $(BUILD_DIR)

clean:
    rm -rf $(BUILD_DIR)
