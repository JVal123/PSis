// File: shared/definitions.h
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MAX_PLAYERS 8 // Maximum number of players that can connect to the game
#define GRID_SIZE 20 // Size of the grid where the game takes place

// Socket addresses for server and client communication
#define SOCKET_ADDRESS_SERVER_C "tcp://*:5556"
#define SOCKET_ADDRESS_SERVER_D "tcp://*:5555"
//#define SOCKET_ADDRESS_PARENT "tcp://*:5557"
#define SOCKET_ADDRESS_CLIENT  "tcp://localhost:5556"
#define SOCKET_ADDRESS_DISPLAY "tcp://localhost:5555"
//#define SOCKET_ADDRESS_CHILD "tcp://localhost:5557"

//#define MAX_ALIENS ((GRID_SIZE-2) * (GRID_SIZE-2) / 3) // Maximum number of aliens allowed in the game
#define MAX_ALIENS 1

// Structure to represent an astronaut's data
typedef struct {
    char id;              // Unique identifier for the astronaut (e.g., 'A', 'B', etc.)
    int x, y;             // Position (x,y) of the astronaut on the grid
    int score;            // Astronaut's score
    time_t last_zap;      // Timestamp of the last time the astronaut used the zap ability
    time_t end_stun;      // Timestamp indicating when the astronaut will recover from stun
    int token;
} Astronaut;

// Structure to represent the position of an alien
typedef struct {
    int x, y; // Position (x,y) of the alien on the grid
} Alien; 

// Different types of messages exchanged between the client and server
typedef enum {
    ASTRONAUT_CONNECT,    // Message for when an astronaut connects to the game
    ASTRONAUT_DISCONNECT, // Message for when an astronaut disconnects from the game
    ASTRONAUT_MOVEMENT,   // Message for astronaut movement updates
    ASTRONAUT_ZAP,        // Message for when an astronaut uses the zap ability
    REFUSED_CONNECTION,    // Message indicating the connection is refused due to reaching max players
    ALIENS_UPDATE
} MessageType;

// Structure to represent a message being sent between client and server
typedef struct {
    MessageType type;     // Type of the message (see MessageType enum)
    char astronaut_id;    // ID of the astronaut sending the message (if applicable)
    int x, y;             // Coordinates (x, y) for movement or other actions
    int score;            // Astronaut's score (if applicable)
    Alien aliens[MAX_ALIENS];
    bool occupied[20][20];
    int num_aliens;
    int token;  //token created by the server when an austronaut connects in order to avoid erroneous connections
} Message;

// Structure to represent an update message containing grid data (arena and score)
typedef struct {
    char arena_grid[GRID_SIZE][GRID_SIZE]; // Grid representing the arena window
    char score_grid[GRID_SIZE][GRID_SIZE]; // Grid representing the score window
} Update_Message;

#endif //DEFINITIONS_H