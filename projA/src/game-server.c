#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include "definitions.h"

Astronaut astronauts[MAX_PLAYERS]; // Array to store astronaut data for each player
int astronaut_counter; // Counter to keep track of the number of astronauts
Alien aliens[MAX_ALIENS]; // Array to store alien data
int num_aliens; // Counter to keep track of the number of aliens currently in the game
bool occupied[20][20]; // Grid to keep track of aliens position, when initializing them and updating their position


// Pointers to the arena and score windows for ncurses UI
WINDOW *arena_win;
WINDOW *score_win;

void define_initial_position(Astronaut *astronaut) {
    
    // Define initial positions based on the astronaut's id and the astronaut_counter
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

    // Set additional fields for the astronaut
    astronaut->score = 0;
    astronaut->active = 1;
    astronauts->last_zap = 0;
    astronauts->end_stun = 0;
}

void initialize_astronaut(char *identifiers, Message *msg) {

    astronauts[astronaut_counter].id = *identifiers; // Assign an identifier to the astronaut's id field

    define_initial_position(&astronauts[astronaut_counter]); // Defines initial position of astronaut and initialize its other fields

    // Update message information with the newly created astronaut
    msg->astronaut_id = astronauts[astronaut_counter].id;
    msg->x = astronauts[astronaut_counter].x;
    msg->y = astronauts[astronaut_counter].y;
    msg->score = astronauts[astronaut_counter].score;

    // Increment the astronaut counter to keep track of how many astronauts have been initialized
    astronaut_counter += 1; 
    
}

void initialize_aliens() {
    num_aliens = MAX_ALIENS;
    // Initialize occupied array
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            occupied[i][j] = false; // Explicitly setting to false
        }
    }

    for (int i = 0; i < num_aliens; i++) {
        int x, y;

        do {
            x = rand() % 16 + 3; // Column 3 to 18 (1-based)
            y = rand() % 16 + 3; // Row 3 to 18 (1-based)
        } while (occupied[y - 1][x - 1]); // Retry if position is occupied

        aliens[i].x = x;
        aliens[i].y = y;
        occupied[y - 1][x - 1] = true;
    }
}

void update_aliens() {
    for (int i = 0; i < num_aliens; i++) {
        int direction = rand() % 4;
        int new_x = aliens[i].x;
        int new_y = aliens[i].y;

        switch (direction) {
            case 0: if (new_y > 3) new_y--; break; // Move up
            case 1: if (new_y < 18) new_y++; break; // Move down
            case 2: if (new_x > 3) new_x--; break; // Move left
            case 3: if (new_x < 18) new_x++; break; // Move right
        }

        if (!occupied[new_y - 1][new_x - 1]) {
            occupied[aliens[i].y - 1][aliens[i].x - 1] = false;
            aliens[i].x = new_x;
            aliens[i].y = new_y;
            occupied[new_y - 1][new_x - 1] = true;
        }
    }

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            occupied[i][j] = false;
        }
    }
    for (int i = 0; i < num_aliens; i++) {
        occupied[aliens[i].y - 1][aliens[i].x - 1] = true;
    }
}

void copy_aliens_to_struct(AlienData *aliens_data) {
    // Update the global aliens array
    for(int i = 0; i < MAX_ALIENS; i++) {
        aliens_data->aliens[i] = aliens[i]; // Copy received data
    }
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            aliens_data->occupied[i][j] = occupied[i][j];
        }
    }
    aliens_data->num_aliens = num_aliens;
}

void copy_aliens_from_struct(AlienData *aliens_data) {
    // Update the global aliens array
    for(int i = 0; i < MAX_ALIENS; i++) {
        aliens[i] = aliens_data->aliens[i]; // Copy received data
    }
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            occupied[i][j] = aliens_data->occupied[i][j];
        }
    }
    num_aliens = aliens_data->num_aliens;
}

Astronaut *find_astronaut_by_id(Astronaut astronauts[], char id) {
    // Iterate through all the astronauts to find the one with the matching id
    for (int i = 0; i < astronaut_counter; i++) {
        if (astronauts[i].id == id) {
            return &astronauts[i];  // Return pointer to the matching astronaut
        }
    }
    return NULL;  // Return NULL if no match is found
}

void update_astronaut(Astronaut *astronaut, Message *msg) {

    // Check if the astronaut is not stunned
    if (difftime(time(NULL), astronaut->end_stun) > 0){

        // Check if the astronaut's movement is within the allowed boundaries of the grid
        if ((msg->y == -1 && astronaut->y > 3) || (msg->y == 1 && astronaut->y < 18) || 
        (msg->x == -1 && astronaut->x > 3) || (msg->x == 1 && astronaut->x < 18)) { 

            // Vertical movement for astronauts with these IDs ('A', 'C', 'E', 'G')
            if (astronaut->id == 'A' || astronaut->id == 'C' || astronaut->id == 'E' || astronaut->id == 'G') {
            astronaut->y += msg->y;
            }
            // Horizontal movement for astronauts with these IDs ('B', 'D', 'F', 'H')
            else if (astronaut->id == 'B' || astronaut->id == 'D' || astronaut->id == 'F' || astronaut->id == 'H') {
                astronaut->x += msg->x;
            }
        }
    }
    msg->score = astronaut->score;  // Update the astronaut's score in the message to be sent to the server
}

void zap(Astronaut *astronaut){
    time_t current_time = time(NULL); // Get current time
    if (current_time == (time_t)-1) {
        perror("Error: failed to get current time");
        return;
    }

    // Check if 3 seconds have passed since the last zap and if the astronaut is not stunned
    if (difftime(current_time, astronaut->last_zap) >= 3 && difftime(time(NULL), astronaut->end_stun) > 0) {
        
        // Process zap in vertical and horizontal directions
        for (int i = 3; i < GRID_SIZE-1; i++) {
            int occupied_v = 0; // Flag to track if there is an alien in the vertical zap path (same column as astronaut)
            int occupied_h = 0; // Flag to track if there is an alien in the horizontal zap path (same row as astronaut)

            for (int j = 0; j < num_aliens; j++) {

                // Vertical zap (for specific astronaut IDs)
                if(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G'){
                    if (aliens[j].x == i && aliens[j].y == astronaut->y) {
                        occupied_h = 1;
                        astronaut->score+=1;
                        // Shift aliens in array after one is removed
                        for (int k = j; k < num_aliens; k++) {
                            aliens[k] = aliens[k + 1];
                        }
                        num_aliens--;
                        j--;
                    }
                }
                // Horizontal zap (for other astronaut IDs)
                else if(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H'){
                    if (aliens[j].x == astronaut->x && aliens[j].y == i) {
                        occupied_v = 1;
                        astronaut->score+=1;
                        // Shift aliens in array after one is removed
                        for (int k = j; k < num_aliens; k++) {
                            aliens[k] = aliens[k + 1];
                        }
                        num_aliens--;
                        j--;
                    }
                }   

            }
            // Draw the zap path for horizontal direction
            if (!occupied_h&&(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G')) {
                mvwaddch(arena_win, astronaut->y, i, '-');
            }
            // Draw the zap path for vertical direction
            if (!occupied_v&&(astronaut->id=='B'||astronaut->id=='D'||astronaut->id=='F'||astronaut->id=='H')) {
                mvwaddch(arena_win, i, astronaut->x,  '|');
            }
        }
        wrefresh(arena_win); // Refresh the arena window to reflect changes

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

        astronaut->last_zap = current_time; // Update last zap time
    }
}

void draw() {

    werase(arena_win); // Clear the arena window
    box(arena_win, 0, 0); // Redraw the border

    // Draw the astronauts
    for (int i = 0; i <astronaut_counter; i++) {
        mvwaddch(arena_win, astronauts[i].y, astronauts[i].x, astronauts[i].id);
    }

    // Draw the aliens as '*' in the arena window
    for (int i = 0; i < num_aliens; i++) {
        mvwaddch(arena_win, aliens[i].y, aliens[i].x, '*');
    }

    wrefresh(arena_win); // Refresh the arena window to reflect the updates

    werase(score_win); // Clear the score window
    box(score_win, 0, 0); // Redraw the border
    
    mvwprintw(score_win, 1, 2, "SCORE"); // Display score header

    // Display astronaut scores
    for (int i = 0; i < astronaut_counter; i++) {
        mvwprintw(score_win, i + 3, 2, "%c - %d", astronauts[i].id, astronauts[i].score);
    }

    wrefresh(score_win); // Refresh the score window to reflect the updates
}

void create_display(Update_Message *display_msg){
    // Loop through each cell in the grid
    for(int i=0; i<GRID_SIZE; i++){
        for(int j=0; j<GRID_SIZE; j++){
            // Store in display message structure the character at position (j, i)
            display_msg->arena_grid[i][j]=mvwinch(arena_win,i,j);
            display_msg->score_grid[i][j]=mvwinch(score_win,i,j);
        }
    }
}

void game_over(){
    Astronaut *highest_score_astronaut = NULL; // Declare a pointer for the astronaut with the highest score
    int highest_score = -1; // Initialize a variable to store the highest score
    int tie_count = 0; // Track how many astronauts have the highest score

    // Loop through all astronauts to find the one with the highest score
    for (int i = 0; i < astronaut_counter; i++) {
        if (astronauts[i].score > highest_score) {
            highest_score = astronauts[i].score;
            highest_score_astronaut = &astronauts[i];
        }
        else if (astronauts[i].score == highest_score) {
            tie_count++; // Increment tie count if another astronaut has the same score
        }
    }

    // Check if any astronaut was foun
    if (highest_score_astronaut == NULL) {
        mvwprintw(score_win, 18, 2, "Game over.");
        mvwprintw(score_win, 19, 2, "No players or scores to display.");
        wrefresh(score_win);
        return;
    }

    // Display the game over message and the winner's ID
    mvwprintw(score_win, 18, 2, "Game over.");

    if (tie_count > 0) {
        mvwprintw(score_win, 19, 2, "It's a tie!");
    } else {
        mvwprintw(score_win, 19, 2, "Player %c wins!", highest_score_astronaut->id);
    }
    
    wrefresh(score_win);
}

void disconnect_astronaut(Message *msg) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (astronauts[i].id == msg->astronaut_id && astronauts[i].active) {
            astronauts[i].active = 0; // Mark astronaut as inactive

            //Dields are cleared for robustness
            astronauts[i].id = 0;
            astronauts[i].x = 0;
            astronauts[i].y = 0;
            astronauts[i].score = 0;

            return;
        }
    }
}


void game_loop(void *context) {
    // Create a ZeroMQ REP (reply) socket for communication with the client
    void *socket = zmq_socket(context, ZMQ_REP);
    if (!socket) {
        perror("Failed to create REP socket");
        exit(1);
    }
    int bind_status = zmq_bind(socket, SOCKET_ADDRESS_SERVER_C);
    if (bind_status != 0) {
        perror("Failed to bind REP socket");
        zmq_close(socket);
        exit(1);
    }

    // Create a ZeroMQ PUB (publish) socket for broadcasting the game state
    void *pub_socket = zmq_socket(context, ZMQ_PUB);
    if (!pub_socket) {
        perror("Failed to create PUB socket");
        exit(1);
    }
    bind_status = zmq_bind(pub_socket, SOCKET_ADDRESS_SERVER_D);
    if (bind_status != 0) {
        perror("Failed to bind PUB socket");
        zmq_close(pub_socket);
        exit(1);
    }
    usleep(10000); // Small delay to allow binding to complete

    void *aliens_socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(aliens_socket, SOCKET_ADDRESS_PARENT);

    Message msg; // Declare a message structure to receive data from the client
    Update_Message display_msg; // Declare a message for the display updates
    astronaut_counter = 0; // Initialize astronaut counter
    Astronaut *selected_astronaut; // Declare a pointer to store the currently selected astronaut
    char identifiers[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}; // Array of identifiers used to assign unique IDs to astronauts (A-H)

    // Structure to receive alien data
    AlienData aliens_data;

    // Main game loop
    while (1) {

        // Receive a message from the client (blocking call)
        int recv_status = zmq_recv(socket, &msg, sizeof(msg), 0);
        if (recv_status == -1) {
            perror("Failed to receive message");
            break;  // Or handle the error more gracefully, like retrying or quitting the loop
        }        
        
        // Handle the received message type
        switch (msg.type) {
            case ASTRONAUT_CONNECT:
                if (astronaut_counter >= MAX_PLAYERS) {
                    msg.type = REFUSED_CONNECTION;
                    zmq_send(socket, &msg, sizeof(msg), 0);
                    continue; // Skip the remaining code for this iteration
                }
                // When an astronaut connects, initialize their data
                initialize_astronaut(&identifiers[astronaut_counter], &msg);
                break;

            case ASTRONAUT_MOVEMENT:
                // Update astronaut movement based on received data
                selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id); // Find the astronaut by ID in the astronauts array
                update_astronaut(selected_astronaut, &msg);

                draw(); // Redraw the windows 
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0); // Publish the update
                break;

            case ASTRONAUT_ZAP:
                // Handle an astronaut zapping another astronaut or target
                selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id); // Find the astronaut by ID in the astronauts array
                zap(selected_astronaut);
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0); // Publish the update
                usleep(500000); // 0.5s delay to show the zap line

                draw(); // Redraw the windows
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0); // Publish the update
                break;

            case ASTRONAUT_DISCONNECT:
                // Handle astronaut disconnection (e.g., cleanup resources, remove from list)
                disconnect_astronaut(&msg);
                break;
            default:
                // Handle unexpected or unknown message types (logging, errors, etc.)
                break;
        }

        copy_aliens_to_struct(&aliens_data);
        zmq_send(aliens_socket, &aliens_data, sizeof(aliens_data), 0); // Reply with alien data updated
        zmq_recv(aliens_socket, &aliens_data, sizeof(aliens_data), 0);
        copy_aliens_from_struct(&aliens_data);

        // Reply back to the client
        zmq_send(socket, &msg, sizeof(msg), 0);
        usleep(10000);

        // Check if all aliens are eliminated (game over condition)
        if(num_aliens == 0){
            game_over();
            create_display(&display_msg); // Create the final display message
            zmq_send(pub_socket, &display_msg, sizeof(display_msg), 0); // Publish final game state
            usleep(5000000); // Wait for 5 seconds before exiting
            break;
        } else if (num_aliens < 0) {
            // Invalid number of aliens
            break;
        }
    }

    // Close the ZeroMQ sockets when exiting the loop
    zmq_close(socket);
    zmq_close(pub_socket);
}

int main() {
    // Initialize the ncurses library for terminal handling
    if (initscr() == NULL) {
        perror("Error initializing ncurses");
        return 1;
    }
    noecho();
    cbreak();
    curs_set(FALSE);

    // Write numbers along the grid's borders (for labeling)
    for (int i = 1; i <= GRID_SIZE; i++) {
        mvprintw(i+1, 0, "%d", i % 10);
        mvprintw(0, i+1, "%d", i % 10);
    }

    // Create windows for the arena and score display
    arena_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, 1);
    if (arena_win == NULL) {
        perror("Error creating arena window");
        endwin();
        return 1;
    }
    box(arena_win, 0, 0);
    score_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, GRID_SIZE + 5);
    if (score_win == NULL) {
        perror("Error creating score window");
        delwin(arena_win);  // Clean up the previously created window
        endwin();
        return 1;
    }
    box(score_win, 0, 0);

    // Refresh and windows to reflect changes
    refresh();
    wrefresh(arena_win);
    wrefresh(score_win);

    
    srand(time(NULL));

    // Create a ZeroMQ context for communication
    void *context = zmq_ctx_new(); 
    if (context == NULL) {
        perror("Error creating ZeroMQ context");
        delwin(arena_win);
        delwin(score_win);
        endwin();
        return 1;
    }

    // Create a fork for alien management
    int process = fork();

    if (process < 0) {
        perror("Fork failed");
        endwin();
        return 1;
    }

    if (process == 0) {
        // Child process: Alien management
        void *socket = zmq_socket(context, ZMQ_REQ); // REP socket
        zmq_connect(socket, SOCKET_ADDRESS_CHILD); // Bind to a port

        initialize_aliens(); // Initialize aliens in child process
        AlienData aliens_data;
        copy_aliens_to_struct(&aliens_data);

        zmq_send(socket, &aliens_data, sizeof(aliens_data), 0);
        zmq_recv(socket, &aliens_data, sizeof(aliens_data), 0);
        copy_aliens_from_struct(&aliens_data);

        while (1) {
            // Send request from the parent
            update_aliens(); //updates the positions of aliens
            copy_aliens_to_struct(&aliens_data);
            zmq_send(socket, &aliens_data, sizeof(aliens_data), 0);
            sleep(1);        // Update aliens every second
            zmq_recv(socket, &aliens_data, sizeof(aliens_data), 0);
            copy_aliens_from_struct(&aliens_data);
        }

        zmq_close(socket);
        _exit(0); // Exit child process safely

    } else {
        // Parent process: Enter the game loop (handling events and game logic)
        game_loop(context); // Pass socket to game loop

        // Clean up the ZeroMQ context after the game loop ends
        zmq_ctx_destroy(context);

        wait(NULL); // Wait for child process to terminate
        endwin();  // End the ncurses session and restore terminal settings
        return 0;
    }
}
