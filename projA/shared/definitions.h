// File: shared/definitions.h
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MAX_PLAYERS 8
#define GRID_SIZE 20
#define SOCKET_ADDRESS_SERVER_C "tcp://*:5556"
#define SOCKET_ADDRESS_SERVER_D "tcp://*:5555"
#define SOCKET_ADDRESS_CLIENT  "tcp://localhost:5556"
#define SOCKET_ADDRESS_DISPLAY "tcp://localhost:5555"

//#define MAX_ALIENS (GRID_SIZE * GRID_SIZE / 3)
#define MAX_ALIENS 3

typedef struct {
    char id;
    int x, y;
    int score;
    int active;
    time_t last_zap, end_stun;
} Astronaut;

typedef struct {
    int x, y;
} Alien;

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
    //UpdateType update_type;
    char astronaut_id;
    int x, y, score;
    //Alien alien[MAX_ALIENS]
} Message;

// Update Structure
typedef struct {
    char arena_grid[GRID_SIZE][GRID_SIZE], score_grid[GRID_SIZE][GRID_SIZE];
} Update_Message;

#endif //DEFINITIONS_H