#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include "definitions.h"
#include <unistd.h>

// Global variables
WINDOW *arena_win;
WINDOW *score_win;
Astronaut astronauts[MAX_PLAYERS];
int astronaut_count = 0;

void draw(Update_Message *msg) {

    int found = 0;

    // Check if the astronaut already exists in the array
    for (int i = 0; i < astronaut_count; i++) {
        if (astronauts[i].id == msg->astronaut_id) {
            // Overwrite the current position with a space
            //mvwaddch(arena_win, astronauts[i].x, astronauts[i].y, ' ');

            // Update position to the new coordinates
            astronauts[i].x = msg->x;
            astronauts[i].y = msg->y;
            astronauts[i].score = msg->score;
            found = 1;
            break;
        }
    }

    //wrefresh(arena_win);

    // If astronaut is not found, add them to the array
    if (!found && astronaut_count < MAX_PLAYERS) {
        astronauts[astronaut_count].x = msg->x;
        astronauts[astronaut_count].y = msg->y;
        astronauts[astronaut_count].id = msg->astronaut_id;
        astronauts[astronaut_count].score = 0;
        astronaut_count++;
    }

    // Redraw the arena window with all astronauts
    werase(arena_win); // Clear the arena window
    box(arena_win, 0, 0); // Redraw the border

    for (int i = 0; i < astronaut_count; i++) {
        // Place the astronaut at the updated position
        mvwaddch(arena_win, astronauts[i].x, astronauts[i].y, astronauts[i].id);
    }

    
    // Draw the aliens as '*' in the arena window
    for (int i = 0; i < msg->alien_count; i++) {
        mvwaddch(arena_win, msg->aliens[i].x, msg->aliens[i].y, '*');
    }

    wrefresh(arena_win);
}

void draw_scores(){
    werase(score_win);
    box(score_win, 0, 0);
    mvwprintw(score_win, 1, 2, "SCORE");

    for (int i = 0; i < astronaut_count; i++) {
        mvwprintw(score_win, i + 3, 2, "%c - %d", astronauts[i].id, astronauts[i].score);
    }

    wrefresh(score_win);
}

void zap(Astronaut *astronaut, Update_Message*msg){
    
    for (int i = 3; i < GRID_SIZE-1; i++) {
        // Skip positions where aliens ('*') are
        int occupied_v = 0;
        int occupied_h = 0;

        for (int j = 0; j < msg->alien_count; j++) {
            if(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G'){
                if (msg->aliens[j].y == astronaut->y && msg->aliens[j].x == i) {
                    occupied_v = 1;
                }
            }
            if(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H'){
                if (msg->aliens[j].x == astronaut->x && msg->aliens[j].y == i) {
                    occupied_h = 1;
                }
            }
            
        }
        if (!occupied_h&&(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H')) {
            mvwaddch(arena_win, astronaut->x, i, '-');
        }
        if (!occupied_v&&(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G')) {
            mvwaddch(arena_win, i, astronaut->y, '|');
        }
    }
    wrefresh(arena_win);
}

Astronaut *find_astronaut_by_id(Astronaut astronauts[], char id) {
    for (int i = 0; i < astronaut_count; i++) {
        if (astronauts[i].id == id) {
            return &astronauts[i];  // Return pointer to the matching astronaut
        }
    }
    return NULL;  // Return NULL if no match is found
}

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

    while (1) {
        zmq_recv(subscriber, &msg, sizeof(msg), 0); // Receive a message

        // Check if the message type is OUTER_SPACE_UPDATE
        if (msg.update_type == MOVEMENT) {
            draw(&msg);
            draw_scores();
        }

        // Check if the message type is OUTER_SPACE_UPDATE
        if (msg.update_type == ZAP) {
            //mvwaddch(arena_win, 9, 9, '|');
            //wrefresh(arena_win);
            //usleep(500000);
            Astronaut *selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id);
            zap(selected_astronaut, &msg);
        }
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    endwin();

    return 0;
}