# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude  # Compiler flags: warnings, debug info, include path
LDFLAGS = -lglfw -lGL -ldl -lm       # Linker flags (libraries)

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
SHADER_DIR = shaders

# List of source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
# List of object files (replace .c with .o and src/ with build/)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target (build the executable)
all: $(BUILD_DIR)/MyGame

# Rule to build the executable
$(BUILD_DIR)/MyGame: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Rule to compile a .c file to a .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D) # Create the build directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target (remove object files and executable)
clean:
	rm -rf $(BUILD_DIR)

#tell make that "all" and "clean" are not files
.PHONY: all clean