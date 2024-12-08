#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "definitions.h"


// Global state
Astronaut astronauts[MAX_PLAYERS];
Alien aliens[MAX_ALIENS];
int num_aliens;
char identifiers[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};


void define_initial_position(int *counter, Astronaut *astronaut) {
    // Astronauts initial positions are defined based on their id

    // Astronaut A
    if (*counter == 0) {
        astronaut->x = 1;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut B
    else if (*counter == 1) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 1;
    }
    // Astronaut C
    else if (*counter == 2) {
        astronaut->x = 20;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut D
    else if (*counter == 3) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 20;
    }
    // Astronaut E
    else if (*counter == 4) { 
        astronaut->x = 2;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut F
    else if (*counter == 5) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 2;
    }
    // Astronaut G
    else if (*counter == 6) {
        astronaut->x = 19;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut H
    else if (*counter == 7) {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 19;
    }

    // The rest of the astronaut fields are also updated
    astronaut->score = 0;
    astronaut->active = 1;
}



void initialize_astronaut(int *counter, char *identifiers, Message *msg) {

    astronauts[*counter].id = *identifiers; // Assigning astronaut ID

    define_initial_position(counter, &astronauts[*counter]); // Defines initial position of astronaut and other fields

    // Update message information with the newly created astronaut
    msg->astronaut_id = astronauts[*counter].id;
    msg->x = astronauts[*counter].x;
    msg->y = astronauts[*counter].y;
    msg->score = astronauts[*counter].score;

    *counter += 1;  // Increment the original counter using the pointer
}



void initialize_aliens() {
    // Place aliens randomly
    num_aliens = MAX_ALIENS;
    for (int i = 0; i < num_aliens; i++) {
        aliens[i].x = rand() % (GRID_SIZE-4)+3; //alterar pois os aliens só podem estar posições específicas
        aliens[i].y = rand() % (GRID_SIZE-4)+3;
    }
}


// Updates alien positions randomly
/*void update_aliens() {
    for (int i = 0; i < num_aliens; ++i) {
        int direction = rand() % 4;
        switch (direction) {
            case 0: if (aliens[i].x > 0) aliens[i].x--; break;  // Move up
            case 1: if (aliens[i].x < GRID_SIZE - 1) aliens[i].x++; break; // Move down
            case 2: if (aliens[i].y > 0) aliens[i].y--; break;  // Move left
            case 3: if (aliens[i].y < GRID_SIZE - 1) aliens[i].y++; break; // Move right
        }
    }
}*/



Astronaut *find_astronaut_by_id(Astronaut astronauts[], int num_astronauts, char id) {
    for (int i = 0; i < num_astronauts; i++) {
        if (astronauts[i].id == id) {
            return &astronauts[i];  // Return pointer to the matching astronaut
        }
    }
    return NULL;  // Return NULL if no match is found
}



void update_astronaut(Astronaut *astronaut, Message *msg) {

    //printf("\rSelected Astronaut: %c\n", astronaut->id);
    //printf("\rAstronaut (x, y) before update: (%i, %i)\n", astronaut->x, astronaut->y);

    if ((msg->y == -1 && astronaut->y > 3) || (msg->y == 1 && astronaut->y < 18) || 
        (msg->x == -1 && astronaut->x > 3) || (msg->x == 1 && astronaut->x < 18)) { 
        if (astronaut->id == 'A' || astronaut->id == 'C' || astronaut->id == 'E' || astronaut->id == 'G') {
        astronaut->y += msg->y; //Only changes the position verticaly
        }
        else if (astronaut->id == 'B' || astronaut->id == 'D' || astronaut->id == 'F' || astronaut->id == 'H') {
            astronaut->x += msg->x; //Only changes the position horizontaly
        }
    }

    msg->score = astronaut->score;

    //printf("\rAstronaut (x, y) after update: (%i, %i)\n", astronaut->x, astronaut->y);
}



void display_astronaut(Astronaut *astronaut, Update_Message *display_msg){
    
    //display_msg->type = OUTER_SPACE_UPDATE;
    display_msg->astronaut_id = astronaut->id;
    display_msg->x = astronaut->x;
    display_msg->y = astronaut->y;
    display_msg->update_type = MOVEMENT;
    display_msg->alien_count = num_aliens;
    display_msg->score = astronaut->score;

    // Copy the updated aliens to the display message
    for (int i = 0; i < num_aliens; i++) {
        display_msg->aliens[i] = aliens[i];
    }
}


// checks which aliens were zapped and removes them from the alien array
void zap(Astronaut *astronaut){

    for (int i = 3; i < GRID_SIZE-1; i++) {
        for (int j = 0; j < num_aliens; j++) {
            //vertical zap
            if(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G'){
                if (aliens[j].x == i && aliens[j].y == astronaut->y) {
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
                    for (int k = j; k < num_aliens; k++) {
                        astronaut->score+=1;
                        aliens[k] = aliens[k + 1];
                    }
                    num_aliens--;
                    j--;
                }
            }   

        }
    } 
}

// creates the message to display the zap line
void display_zap(Astronaut *astronaut, Update_Message*display_msg){
    
    display_msg->update_type = ZAP;
    display_msg->astronaut_id = astronaut->id;
}

void game_loop(void *context) {
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, SOCKET_ADDRESS_SERVER_C);

    void *pub_socket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(pub_socket, "tcp://*:5555");
    usleep(1000000);

    Message msg;
    Update_Message display_msg;
    int astronaut_counter = 0;

    while (1) {
        zmq_recv(socket, &msg, sizeof(msg), 0);
        Astronaut *selected_astronaut = find_astronaut_by_id(astronauts, astronaut_counter, msg.astronaut_id);

        switch (msg.type) {
            case ASTRONAUT_CONNECT:
                // Add astronaut
                initialize_astronaut(&astronaut_counter, &identifiers[astronaut_counter], &msg);
                break;
            case ASTRONAUT_MOVEMENT:
                // Update astronaut movement
                update_astronaut(selected_astronaut, &msg);
                display_astronaut(selected_astronaut, &display_msg);
                usleep(100000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);
                break;
            case ASTRONAUT_ZAP: // as it is right now, the score will only be updated when the played moves again
                // Handle zapping
                display_zap(selected_astronaut, &display_msg);
                usleep(100000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0);       
                zap(selected_astronaut);
                break;
            case ASTRONAUT_DISCONNECT:
                // Remove astronaut
                break;
            /*case OUTER_SPACE_UPDATE:
                // No action needed for updates here
                break;*/
            default:
                // Handle unexpected message types
                break;
        }

        zmq_send(socket, &msg, sizeof(msg), 0);
        usleep(100000); // Slow down loop (100ms)
    }

    zmq_close(socket);
    zmq_close(pub_socket);
}

int main() {
    //initscr();
    //noecho();
    //cbreak();

    srand(time(NULL));
    initialize_aliens();

    void *context = zmq_ctx_new();

    game_loop(context);
    
    zmq_ctx_destroy(context);
    //endwin();
    return 0;
}
