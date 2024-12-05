# File: Makefile

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -lncurses -lzmq -Ishared
SRC_DIR = src
BUILD_DIR = build
SHARED_DIR = shared

# Targets
TARGETS = game-server astronaut-client outer-space-display

# Source Files
GAME_SERVER_SRC = $(SRC_DIR)/game-server.c
ASTRONAUT_CLIENT_SRC = $(SRC_DIR)/astronaut-client.c
OUTER_SPACE_DISPLAY_SRC = $(SRC_DIR)/outer-space-display.c
SHARED_HEADER = $(SHARED_DIR)/definitions.h

# Output Files
GAME_SERVER = $(BUILD_DIR)/game-server
ASTRONAUT_CLIENT = $(BUILD_DIR)/astronaut-client
OUTER_SPACE_DISPLAY = $(BUILD_DIR)/outer-space-display

# All Target
all: $(TARGETS)

game-server: $(GAME_SERVER_SRC) $(SHARED_HEADER)
	$(CC) $(GAME_SERVER_SRC) -o $(GAME_SERVER) $(CFLAGS)

astronaut-client: $(ASTRONAUT_CLIENT_SRC) $(SHARED_HEADER)
	$(CC) $(ASTRONAUT_CLIENT_SRC) -o $(ASTRONAUT_CLIENT) $(CFLAGS)

outer-space-display: $(OUTER_SPACE_DISPLAY_SRC) $(SHARED_HEADER)
	$(CC) $(OUTER_SPACE_DISPLAY_SRC) -o $(OUTER_SPACE_DISPLAY) $(CFLAGS)

# Clean Target
clean:
	rm -rf $(BUILD_DIR)/*