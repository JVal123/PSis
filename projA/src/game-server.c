#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "definitions.h"

// Global state
Astronaut astronauts[MAX_PLAYERS];
int astronaut_counter;
Alien aliens[MAX_ALIENS];
int num_aliens;
char identifiers[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
WINDOW *arena_win;
WINDOW *score_win;

//At the moment, astronaut positions are being initialized as 1-based and aliens position as 0-based. Which one is correct?

void define_initial_position(Astronaut *astronaut) {
    // Astronauts initial positions are defined based on their id

    // Astronaut A
    if (astronaut_counter == 0) {
        astronaut->x = 1;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut B
    else if (astronaut_counter == 1) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 1;
    }
    // Astronaut C
    else if (astronaut_counter == 2) {
        astronaut->x = 20;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut D
    else if (astronaut_counter == 3) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 20;
    }
    // Astronaut E
    else if (astronaut_counter == 4) { 
        astronaut->x = 2;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut F
    else if (astronaut_counter == 5) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 2;
    }
    // Astronaut G
    else if (astronaut_counter == 6) {
        astronaut->x = 19;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut H
    else if (astronaut_counter == 7) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 19;
    }

    // The rest of the astronaut fields are also updated
    astronaut->score = 0;
    astronaut->active = 1;
    astronauts->last_zap = 0;
    astronauts->end_stun = 0;
}



void initialize_astronaut(char *identifiers, Message *msg) {

    astronauts[astronaut_counter].id = *identifiers; // Assigning astronaut ID

    define_initial_position(&astronauts[astronaut_counter]); // Defines initial position of astronaut and other fields

    // Update message information with the newly created astronaut
    msg->astronaut_id = astronauts[astronaut_counter].id;
    msg->x = astronauts[astronaut_counter].x;
    msg->y = astronauts[astronaut_counter].y;
    msg->score = astronauts[astronaut_counter].score;

    astronaut_counter += 1;  // Increment the original counter using the pointer
    
}



void initialize_aliens() {
    // Place aliens randomly
    num_aliens = MAX_ALIENS;
    for (int i = 0; i < num_aliens; i++) {
        aliens[i].x = rand() % (GRID_SIZE-4)+3; //alterar pois os aliens só podem estar posições específicas
        aliens[i].y = rand() % (GRID_SIZE-4)+3;
    }
}



Astronaut *find_astronaut_by_id(Astronaut astronauts[], char id) {
    for (int i = 0; i < astronaut_counter; i++) {
        if (astronauts[i].id == id) {
            return &astronauts[i];  // Return pointer to the matching astronaut
        }
    }
    return NULL;  // Return NULL if no match is found
}



void update_astronaut(Astronaut *astronaut, Message *msg) {

    //printf("\rSelected Astronaut: %c\n", astronaut->id);
    //printf("\rAstronaut (x, y) before update: (%i, %i)\n", astronaut->x, astronaut->y);
    if (difftime(time(NULL), astronaut->end_stun) > 0){
        if ((msg->y == -1 && astronaut->y > 3) || (msg->y == 1 && astronaut->y < 18) || 
        (msg->x == -1 && astronaut->x > 3) || (msg->x == 1 && astronaut->x < 18)) { 
            if (astronaut->id == 'A' || astronaut->id == 'C' || astronaut->id == 'E' || astronaut->id == 'G') {
            astronaut->y += msg->y; //Only changes the position verticaly
            }
            else if (astronaut->id == 'B' || astronaut->id == 'D' || astronaut->id == 'F' || astronaut->id == 'H') {
                astronaut->x += msg->x; //Only changes the position horizontaly
            }
        }
    }
    

    msg->score = astronaut->score;

    //printf("\rAstronaut (x, y) after update: (%i, %i)\n", astronaut->x, astronaut->y);
}



// checks which aliens were zapped and removes them from the alien array
void zap(Astronaut *astronaut){
    time_t current_time = time(NULL); // Get current time

    // Check if 3 seconds have passed since the last zap
    if (difftime(current_time, astronaut->last_zap) >= 3 && difftime(time(NULL), astronaut->end_stun) > 0) {

        for (int i = 3; i < GRID_SIZE-1; i++) {
            int occupied_v = 0;
            int occupied_h = 0;

            for (int j = 0; j < num_aliens; j++) {
                //vertical zap
                if(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G'){
                    if (aliens[j].x == i && aliens[j].y == astronaut->y) {
                        occupied_h = 1;
                        astronaut->score+=1;
                        for (int k = j; k < num_aliens; k++) {
                            aliens[k] = aliens[k + 1];
                        }
                        num_aliens--;
                        j--;
                    }
                }
                // horizontal zap
                else if(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H'){
                    if (aliens[j].x == astronaut->x && aliens[j].y == i) {
                        occupied_v = 1;
                        astronaut->score+=1;
                        for (int k = j; k < num_aliens; k++) {
                            aliens[k] = aliens[k + 1];
                        }
                        num_aliens--;
                        j--;
                    }
                }   

            }
            if (!occupied_h&&(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G')) {
                mvwaddch(arena_win, astronaut->y, i, '-');
            }
            if (!occupied_v&&(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H')) {
                mvwaddch(arena_win, i, astronaut->x,  '|');
            }
        }
        wrefresh(arena_win); 

        // Check for other astronauts to stun them
        for (int k = 0; k < astronaut_counter; k++) {
            if (&astronauts[k] == astronaut) continue; // Skip the astronaut performing the zap

            // Vertical zap: Check for astronauts in the same column
            if ((astronaut->id == 'A' || astronaut->id == 'C' || astronaut->id == 'E' || astronaut->id == 'G') &&
                astronauts[k].y == astronaut->y) {
                astronauts[k].end_stun = current_time + 10; // Stun for 10 seconds
            }

            // Horizontal zap: Check for astronauts in the same row
            if ((astronaut->id == 'B' || astronaut->id == 'D' || astronaut->id == 'F' || astronaut->id == 'H') &&
                astronauts[k].x == astronaut->x) {
                astronauts[k].end_stun= current_time + 10; // Stun for 10 seconds
            }
        }

        astronaut->last_zap = current_time;
    }
}



void draw() {

    // Redraw the arena window with all astronauts
    werase(arena_win); // Clear the arena window
    box(arena_win, 0, 0); // Redraw the border

    for (int i = 0; i <astronaut_counter; i++) {
        // Place the astronaut at the updated position
        mvwaddch(arena_win, astronauts[i].y, astronauts[i].x, astronauts[i].id);
    }

    
    // Draw the aliens as '*' in the arena window
    for (int i = 0; i < num_aliens; i++) {
        mvwaddch(arena_win, aliens[i].y, aliens[i].x, '*');
    }

    wrefresh(arena_win);

    werase(score_win);
    box(score_win, 0, 0);
    mvwprintw(score_win, 1, 2, "SCORE");

    for (int i = 0; i < astronaut_counter; i++) {
        mvwprintw(score_win, i + 3, 2, "%c - %d", astronauts[i].id, astronauts[i].score);
    }

    wrefresh(score_win);
}



void create_display(Update_Message *display_msg){
    for(int i=0; i<GRID_SIZE; i++){
        for(int j=0; j<GRID_SIZE; j++){
            display_msg->arena_grid[i][j]=mvwinch(arena_win,i,j);
            display_msg->score_grid[i][j]=mvwinch(score_win,i,j);
        }
    }
}



void game_over(){

    Astronaut *highest_score_astronaut = NULL;
    int highest_score = -1;
    for (int i = 0; i < astronaut_counter; i++) {
        if (astronauts[i].score > highest_score) {
            highest_score = astronauts[i].score;
            highest_score_astronaut = &astronauts[i]; // Store the pointer to the astronaut with the highest score
        }
    }
    mvwprintw(score_win, 18, 2, "Game over.");
    mvwprintw(score_win, 19, 2, "Player %c wins!", highest_score_astronaut->id);

    wrefresh(score_win);
}



void game_loop(void *context) {
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, SOCKET_ADDRESS_SERVER_C);

    void *pub_socket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(pub_socket, SOCKET_ADDRESS_SERVER_D);
    usleep(10000);

    Message msg;
    Update_Message display_msg;
    astronaut_counter = 0;

    while (1) {
        zmq_recv(socket, &msg, sizeof(msg), 0);
        Astronaut *selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id);

        switch (msg.type) {
            case ASTRONAUT_CONNECT:
                // Add astronaut
                initialize_astronaut(&identifiers[astronaut_counter], &msg);
                break;
            case ASTRONAUT_MOVEMENT:
                // Update astronaut movement
                update_astronaut(selected_astronaut, &msg);
                draw();
                create_display(&display_msg);
                usleep(10000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);
                break;
            case ASTRONAUT_ZAP:
                // Handle zapping
                zap(selected_astronaut);
                create_display(&display_msg);
                //usleep(100000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);
                usleep(500000);
                draw();
                create_display(&display_msg);
                usleep(10000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);
                break;
            case ASTRONAUT_DISCONNECT:
                // Remove astronaut
                break;
            default:
                // Handle unexpected message types
                break;
        }

        zmq_send(socket, &msg, sizeof(msg), 0);
        usleep(10000);

        if(num_aliens == 0){
            game_over();
            create_display(&display_msg);
            zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);
            usleep(5000000);
        }
    }

    zmq_close(socket);
    zmq_close(pub_socket);
}

int main() {
    initscr();
    noecho();
    cbreak();
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

    srand(time(NULL));
    initialize_aliens();

    void *context = zmq_ctx_new();

    game_loop(context);
    
    zmq_ctx_destroy(context);
    endwin();
    return 0;
}
