// File: shared/definitions.h
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MAX_PLAYERS 8
#define GRID_SIZE 20
#define SOCKET_ADDRESS_SERVER "tcp://*:5555"
#define SOCKET_ADDRESS_CLIENT "tcp://localhost:5555"

// Message types
typedef enum {
    ASTRONAUT_CONNECT,
    ASTRONAUT_DISCONNECT,
    ASTRONAUT_MOVEMENT,
    ASTRONAUT_ZAP,
    OUTER_SPACE_UPDATE
} MessageType;

// Message structure
typedef struct {
    MessageType type;
    char astronaut_id;
    int x, y; // Coordinates
    int score;
} Message;

#endif // DEFINITIONS_H