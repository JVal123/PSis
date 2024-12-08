#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include "definitions.h"
#include <unistd.h>

// Global variables
WINDOW *arena_win;
WINDOW *score_win;
char arena_grid[GRID_SIZE][GRID_SIZE];
char score_grid[GRID_SIZE][GRID_SIZE];

int main() {
    
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    

    zmq_connect(subscriber, "tcp://localhost:5555");
 
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);
    
    Update_Message msg;
  
    // initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);

    // write the numbers 
    for (int i = 1; i <= GRID_SIZE; i++) {
        mvprintw(i+1, 0, "%d", i % 10);
        mvprintw(0, i+1, "%d", i % 10);
    }

    // Create windows and draw borders
    arena_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, 1);
    box(arena_win, 0, 0);
    score_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, GRID_SIZE + 5);
    box(score_win, 0, 0);

    refresh();
    wrefresh(arena_win);
    wrefresh(score_win);

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            arena_grid[i][j] = ' ';  // Initialize with spaces
            score_grid[i][j] = ' ';
        }
    }

    while (1) {
        zmq_recv(subscriber, &msg, sizeof(msg), 0); // Receive a message

       for(int i=1; i<GRID_SIZE; i++){
            for(int j=1; j<GRID_SIZE; j++){
                if(arena_grid[i][j]!=msg.arena_grid[i][j]){
                    arena_grid[i][j]=msg.arena_grid[i][j];
                    wmove(arena_win,i,j);
                    waddch(arena_win, arena_grid[i][j]);
                }
                if(score_grid[i][j]!=msg.score_grid[i][j]){
                    score_grid[i][j]=msg.score_grid[i][j];
                    wmove(score_win,i,j);
                    waddch(score_win, score_grid[i][j]);
                }              
            }
        }
        wrefresh(arena_win);
        wrefresh(score_win);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    endwin();

    return 0;
}