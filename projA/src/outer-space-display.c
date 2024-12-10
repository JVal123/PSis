#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include "definitions.h"
#include <unistd.h>

WINDOW *arena_win; // Window for displaying the arena
WINDOW *score_win; // Window for displaying the scores
char arena_grid[GRID_SIZE+1][GRID_SIZE+1];  // Array representing the arena grid
char score_grid[GRID_SIZE+1][GRID_SIZE+1];  // Array representing the score grid

int main() {
    
    // Create a ZeroMQ context
    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("Error creating ZeroMQ context");
        return 1;
    }
    
    // Create a ZeroMQ subscriber socket
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    if (subscriber == NULL) {
        perror("Failed to create SUB socket");
        zmq_ctx_destroy(context);  // Clean up before exiting
        return 1;
    }

    // Connect to the server to receive updates
    if (zmq_connect(subscriber, SOCKET_ADDRESS_DISPLAY) != 0) {
        perror("Failed to connect SUB socket");
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1; 
    }

    // Set the socket to subscribe to all messages
    if (zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0) != 0) {
        perror("Failed to subscribe SUB socket to all messages");
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1;
    }

    Update_Message msg; // Structure to hold the update message
  
    // Initialize the ncurses library for terminal handling
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);

    // Write numbers along the grid's borders (for labeling)
    for (int i = 1; i <= GRID_SIZE; i++) {
        mvprintw(i+1, 0, "%d", i % 10);
        mvprintw(0, i+1, "%d", i % 10);
    }


    // Create windows for the arena display and draw its borders
    arena_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, 1);
    if (arena_win == NULL) {
        perror("Error creating arena window");
        endwin();
        return 1;
    }
    box(arena_win, 0, 0);
    
    // Create windows for the score display and draw its borders
    score_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, GRID_SIZE + 5);
    if (score_win == NULL) {
        perror("Error creating score window");
        delwin(arena_win);
        endwin();
        return 1;
    }
    box(score_win, 0, 0);

    // Refresh windows to reflect changes
    refresh();
    wrefresh(arena_win);
    wrefresh(score_win);

    // Initialize the arena and score grids with empty spaces
    for (int i = 0; i < GRID_SIZE+1; i++) {
        for (int j = 0; j < GRID_SIZE+1; j++) {
            arena_grid[i][j] = ' ';
            score_grid[i][j] = ' '; 
        }
    }

    // Main loop to handle incoming updates
    while (1) {
       // Receive the update message from the publisher (game-server)
        int recv_size = zmq_recv(subscriber, &msg, sizeof(msg), 0);
        if (recv_size == -1) {
            perror("Failed to receive message");
            break;
        }

        // Update the arena and score grids based on the received message
        for(int i=1; i<GRID_SIZE+1; i++){
            for(int j=1; j<GRID_SIZE+1; j++){
                if(arena_grid[i][j]!=msg.arena_grid[i][j]){
                    arena_grid[i][j]=msg.arena_grid[i][j];
                    wmove(arena_win,i,j); // Move cursor to the new position
                    waddch(arena_win, arena_grid[i][j]); // Add the character to the window
                }
                if(score_grid[i][j]!=msg.score_grid[i][j]){
                    score_grid[i][j]=msg.score_grid[i][j];
                    wmove(score_win,i,j); // Move cursor to the new position
                    waddch(score_win, score_grid[i][j]); // Add the character to the window
                }              
            }
        }

        // Refresh windows to reflect the updates
        wrefresh(arena_win);
        wrefresh(score_win);
    }

    // Cleanup and close ZeroMQ socket and context
    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    // End ncurses mode
    endwin();

    return 0;
}