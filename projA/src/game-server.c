#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "definitions.h"

Astronaut astronauts[MAX_PLAYERS]; // Array to store astronaut data for each player
int astronaut_counter; // Counter to keep track of the number of astronauts
Alien aliens[MAX_ALIENS]; // Array to store alien data
int num_aliens; // Counter to keep track of the number of aliens currently in the game
bool occupied[20][20]; // Grid to keep track of aliens position, when initializing them and updating their position
int zapped_index[MAX_ALIENS]; // Array to store the indices of zapped aliens
int zapped_count; // Counter to track the number of zapped aliens

// Pointers to the arena and score windows for ncurses UI
WINDOW *arena_win;
WINDOW *score_win;

void define_initial_position(Astronaut *astronaut, int token) {
    
    // Define initial positions based on the astronaut's id and the astronaut_counter
    // Astronaut A
    if (astronaut->id=='A') {
        astronaut->x = 1;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut B
    else if (astronaut->id=='B') {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 1;
    }
    // Astronaut C
    else if (astronaut->id=='C') {
        astronaut->x = 20;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut D
    else if (astronaut->id=='D') {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 20;
    }
    // Astronaut E
    else if (astronaut->id=='E') { 
        astronaut->x = 2;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut F
    else if (astronaut->id=='F') {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 2;
    }
    // Astronaut G
    else if (astronaut->id=='G') {
        astronaut->x = 19;
        astronaut->y = rand() % 16 + 3;
    }
    // Astronaut H
    else if (astronaut->id=='H') {
        astronaut->x = rand() % 16 + 3;
        astronaut->y = 19;
    }

    // Set additional fields for the astronaut
    astronaut->score = 0;
    astronaut->token = token;
    astronaut->last_zap = 0;
    astronaut->end_stun = 0;
}

void initialize_astronaut(char *identifiers, Message *msg) {

    astronauts[astronaut_counter].id = *identifiers; // Assign an unique identifier to the astronaut's id field

    int token = rand(); // Generate a random token for confirming astronaut identification

    msg->token = token;

    define_initial_position(&astronauts[astronaut_counter], token); // Defines initial position of astronaut and initialize its other fields

    // Update message information with the newly created astronaut
    msg->astronaut_id = astronauts[astronaut_counter].id;
    msg->x = astronauts[astronaut_counter].x;
    msg->y = astronauts[astronaut_counter].y;
    msg->score = astronauts[astronaut_counter].score;

    // Increment the astronaut counter to keep track of how many astronauts have been initialized
    astronaut_counter += 1; 
    
}

bool check_token(Message *msg) { // Function to check if the astronaut token matches

    // Loop through all astronauts
    for (int i = 0; i < MAX_PLAYERS; i++) {
            // Check if the astronaut ID matches
            if (astronauts[i].id == msg->astronaut_id) {
                // If IDs match, compare the tokens for authentication
                return astronauts[i].token == msg->token;
            }
        }
        return false; // Return false if the astronaut ID is not found   
}

void initialize_aliens() {

    num_aliens = MAX_ALIENS; // Set the number of aliens to the maximum allowed

    // Initialize the occupied array to false (no positions are occupied)
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            occupied[i][j] = false;
        }
    }
    // Randomly assign positions for each alien
    for (int i = 0; i < num_aliens; i++) {
        int x, y;
        do {
            x = rand() % 16 + 3;
            y = rand() % 16 + 3; 
        } while (occupied[y - 1][x - 1]); // Retry if position is occupied

        // Assign the random position to the alien and mark it as occupied
        aliens[i].x = x;
        aliens[i].y = y;
        occupied[y - 1][x - 1] = true;
    }
}

void update_aliens() {

    // Loop through each alien to move them in a random direction
    for (int i = 0; i < num_aliens; i++) {

        int direction = rand() % 4; // Randomly choose a direction (0 - up, 1 - down, 2 - left, 3 - right)
        int new_x = aliens[i].x;
        int new_y = aliens[i].y;

        // Move the alien in the selected direction (if possible)
        switch (direction) {
            case 0: if (new_y > 3) new_y--; break; // Move up
            case 1: if (new_y < 18) new_y++; break; // Move down
            case 2: if (new_x > 3) new_x--; break; // Move left
            case 3: if (new_x < 18) new_x++; break; // Move right
        }

        // If the new position is not occupied, update the alien's position
        if (!occupied[new_y - 1][new_x - 1]) {
            occupied[aliens[i].y - 1][aliens[i].x - 1] = false;
            aliens[i].x = new_x;
            aliens[i].y = new_y;
            occupied[new_y - 1][new_x - 1] = true;
        }
    }
}

void copy_aliens_to_struct(Message *aliens_data) {
    aliens_data->num_aliens = num_aliens;  // Copy the number of aliens to the message
    
    // Copy the aliens to the message
    for(int i = 0; i < num_aliens; i++) {
        aliens_data->aliens[i] = aliens[i];
    }
    // Copy the occupied array to the message
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            aliens_data->occupied[i][j] = occupied[i][j];
        }
    }
}

void copy_aliens_from_struct_child(Message *aliens_data) {
    num_aliens = aliens_data->num_aliens; // Set the number of aliens from the message

    // Copy the alien positions from the message to the global aliens array
    for(int i = 0; i < num_aliens; i++) {
        aliens[i] = aliens_data->aliens[i]; // Copy received data
    }

    // Copy the occupied array from the message
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            occupied[i][j] = aliens_data->occupied[i][j];
        }
    }
}

void copy_aliens_from_struct(Message *aliens_data) {
    // Function to copy alien data from message structure to global array (with zapped aliens handling)

    if (zapped_count>0){
        
        int child_num_aliens = aliens_data->num_aliens;

        // If there are zapped aliens, remove them from the aliens list
        for (int i = 0; i < zapped_count; i++) {
            int index_to_remove = zapped_index[i];

            // Shift remaining aliens to the left to remove the zapped one
            for (int j = index_to_remove; j < child_num_aliens; j++) {
                aliens_data->aliens[j] = aliens_data->aliens[j + 1];  // Shift left
            }
            child_num_aliens--; // Decrease the total number of aliens

        }
        // Copy the updated aliens list to the global aliens array
        for (int i = 0; i < num_aliens; i++) {
            aliens[i] = aliens_data->aliens[i];
        }
        aliens_data->num_aliens = num_aliens; // Update the number of aliens
        zapped_count=0; // Reset the zapped count

        // Copy the occupied array from the message
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                occupied[i][j] = aliens_data->occupied[i][j];
            }
        }
    }
    else{
        // If no aliens have been zapped, just update the aliens list normally
        num_aliens = aliens_data->num_aliens;

        // Copy the alien positions from the message to the global aliens array
        for (int i = 0; i < num_aliens; i++) {
            aliens[i] = aliens_data->aliens[i]; // Copy received data
        }

        // Copy the occupied array from the message
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                occupied[i][j] = aliens_data->occupied[i][j];
            }
        }
    }   
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

    zapped_count = 0; // initialize the number of aliens zapped by this zap fire

        // Process zap in vertical and horizontal directions
        for (int i = 3; i < GRID_SIZE-1; i++) {
            int occupied_v = 0; // Flag to track if there is an alien in the vertical zap path (same column as astronaut)
            int occupied_h = 0; // Flag to track if there is an alien in the horizontal zap path (same row as astronaut)
            
            for (int j = 0; j < num_aliens; j++) {

                // Vertical zap (for specific astronaut IDs)
                if(astronaut->id=='A'||astronaut->id=='C'||astronaut->id=='E'||astronaut->id=='G'){
                    if (aliens[j].x == i && aliens[j].y == astronaut->y) {
                        zapped_index[zapped_count] = j;
                        zapped_count+=1;
                        occupied[i-1][astronaut->y-1] = false;

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
                        zapped_index[zapped_count] = j;
                        zapped_count+=1;
                        occupied[astronaut->x-1][i-1] = false;

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
    for(int i=0; i<GRID_SIZE+1; i++){
        for(int j=0; j<GRID_SIZE+1; j++){
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
            tie_count = 0;
        }
        else if (astronauts[i].score == highest_score) {
            tie_count++; // Increment tie count if another astronaut has the same score
        }
    }

    // Check if any astronaut was found
    if (highest_score_astronaut == NULL) {
        mvwprintw(score_win, 18, 2, "Game over.");
        mvwprintw(score_win, 19, 2, "No players or scores to display.");
        wrefresh(score_win);
        return;
    }

    // Display the game over message
    mvwprintw(score_win, 18, 2, "Game over.");

    if (tie_count > 0) {
        mvwprintw(score_win, 19, 2, "It's a tie!");
    } else {
        mvwprintw(score_win, 19, 2, "Player %c wins!", highest_score_astronaut->id);
    }
    
    wrefresh(score_win);
}

void disconnect_astronaut(Message *msg) {

    // Loop through the astronauts array to find the astronaut by id
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (astronauts[i].id == msg->astronaut_id) {
            // If a match is found, shift all subsequent astronauts one position to the left
            for (int j = i; j < MAX_PLAYERS - 1; j++) {
                astronauts[j] = astronauts[j + 1]; 
            }
            astronaut_counter--; // Decrement the astronaut counte
            return;
        }
    }
}

void game_loop(void *context) {
    // Create a ZeroMQ REP (reply) socket for communication with the client
    void *socket = zmq_socket(context, ZMQ_REP);
    if (!socket) {
        endwin();
        perror("Failed to create REP socket");
        exit(1);
    }

    if (zmq_bind(socket, SOCKET_ADDRESS_SERVER_C) != 0) {
        endwin();
        perror("Failed to bind REP socket");
        zmq_close(socket);
        exit(1);
    }

    // Create a ZeroMQ PUB (publish) socket for broadcasting the game state
    void *publisher = zmq_socket(context, ZMQ_PUB);
    if (!publisher) {
        endwin();
        perror("Failed to create PUB socket");
        zmq_close(socket);
        exit(1);
    }

    if (zmq_bind(publisher, SOCKET_ADDRESS_SERVER_D) != 0) {
        endwin();
        perror("Failed to bind PUB socket");
        zmq_close(socket);
        zmq_close(publisher);
        exit(1);
    }
    usleep(10000); // Small delay to allow binding to complete

    Message msg; // Declare a message structure to receive data from the client
    Update_Message display_msg; // Declare a message for the display updates
    Astronaut *selected_astronaut; // Declare a pointer to store the currently selected astronaut
    
    char identifiers[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}; // Array of identifiers used to assign unique IDs to astronauts (A-H)
    char free_id = '\0'; // Initialize a variable to store the first free ID
    
    astronaut_counter = 0; // Initialize astronaut counter
    zapped_count = 0; // Initialize the number of zapped aliens counter

    // Main game loop
    while (1) {

        // Receive a message from the client
        if (zmq_recv(socket, &msg, sizeof(msg), 0) == -1) {
            perror("Failed to receive message");
            break;
        }

        // Handle the received message type
        switch (msg.type) {
            case ASTRONAUT_CONNECT: // Connect a new astronaut

                // Send a REFUSED_CONNECTION message if the maximum number of players has been reached
                if (astronaut_counter >= MAX_PLAYERS) {
                    msg.type = REFUSED_CONNECTION;
                    zmq_send(socket, &msg, sizeof(msg), 0);
                    continue;
                }

                // Find the first available identifier that is not used by any astronaut
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    int id_in_use = 0; // Flag to check if the ID is in use
                    for (int j = 0; j < MAX_PLAYERS; j++) {
                        if (astronauts[j].id == identifiers[i]) {
                            id_in_use = 1; // ID is currently in use
                            break;
                        }
                    }
                    if (!id_in_use) {
                        free_id = identifiers[i];
                        break; // Found the first available ID
                    }
                }

                // When an astronaut connects, initialize their data
                initialize_astronaut(&free_id, &msg);
                break;

            case ASTRONAUT_MOVEMENT: // Update astronaut movement based on received data
                
                if (check_token(&msg) == false) {
                    break;
                }

                selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id); // Find the astronaut by ID in the astronauts array
                update_astronaut(selected_astronaut, &msg);

                draw(); // Redraw the windows 
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(publisher, &display_msg, sizeof(display_msg), 0); // Publish the update
                break;

            case ASTRONAUT_ZAP: // Handle an astronaut zapping another astronaut or target

                if (check_token(&msg) == false) {
                    break;
                }

                selected_astronaut = find_astronaut_by_id(astronauts, msg.astronaut_id); // Find the astronaut by ID in the astronauts array
                zap(selected_astronaut);

                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(publisher, &display_msg, sizeof(display_msg), 0); // Publish the update
                usleep(500000); // 0.5s delay to show the zap line

                draw(); // Redraw the windows
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(publisher, &display_msg, sizeof(display_msg), 0); // Publish the update
                break;

            case ALIENS_UPDATE: // Handle the alien positions updates
                
                copy_aliens_from_struct(&msg);
                draw();
                create_display(&display_msg); // Create the outer-space-display update message
                usleep(10000);
                zmq_send(publisher, &display_msg, sizeof(display_msg), 0); // Publish the update
                break;

            case ASTRONAUT_DISCONNECT:
                // Handle astronaut disconnection

                if (check_token(&msg) == false) {
                        break;
                }
                
                disconnect_astronaut(&msg);
                break;
            default:
                // Handle unexpected or unknown message types
                break;
        }

        // Reply back to the client
        zmq_send(socket, &msg, sizeof(msg), 0);
        usleep(10000);

        // Check if all aliens are eliminated (game over condition)
        if(num_aliens == 0){
            game_over();
            create_display(&display_msg); // Create the final display message
            zmq_send(publisher, &display_msg, sizeof(display_msg), 0); // Publish final game state
            usleep(5000000); // Wait for 5 seconds before exiting
            break;

        } else if (num_aliens < 0) {
            // Invalid number of aliens
            break;
        }
    }

    // Close the ZeroMQ sockets when exiting the loop
    zmq_close(socket);
    zmq_close(publisher);
}

int main() {
    // Initialize the ncurses library for terminal handling
    if (initscr() == NULL) {
        perror("Error initializing ncurses");
        exit(1);
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
        endwin();
        perror("Error creating arena window");
        exit(1);
    }
    box(arena_win, 0, 0);
    score_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, GRID_SIZE + 5);
    if (score_win == NULL) {
        endwin();
        perror("Error creating score window"); 
        exit(1);
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
        endwin();
        perror("Error creating ZeroMQ context");
        exit(1);
    }

    // Create a fork for alien management
    int process = fork();

    if (process < 0) {
        endwin();
        perror("Fork failed");
        exit(1);
    }

    if (process == 0) {
        
        void *socket = zmq_socket(context, ZMQ_REQ); // Create a ZeroMQ request (REQ) socket
        if (!socket) {
            perror("Failed to create REP socket");
            exit(1);
        }
    
        if (zmq_connect(socket, SOCKET_ADDRESS_CLIENT) != 0) {  // Connect to the server address    
            perror("Failed to connect SUB socket");
            zmq_close(socket);
            zmq_ctx_destroy(context);
            exit(1);
        }
    
        // Initialize the ALIENS_UPDATE message
        Message aliens_data = {0};
        initialize_aliens(); // Initialize aliens
        copy_aliens_to_struct(&aliens_data);
        aliens_data.type = ALIENS_UPDATE;

        // Send initial alien data to the server
        zmq_send(socket, &aliens_data, sizeof(aliens_data), 0);

        // Loop to update aliens at a rate of 1 place/s
        while (1) {
            zmq_recv(socket, &aliens_data, sizeof(aliens_data), 0); // Receive the server's response

            copy_aliens_from_struct_child(&aliens_data); // Copy the received alien data into the local data structure
            update_aliens(); // Update the position of the aliens
            copy_aliens_to_struct(&aliens_data); //  Copy the updated alien positions back into the message structure
            
            zmq_send(socket, &aliens_data, sizeof(aliens_data), 0); // Send the updated alien data back to the server
            sleep(1);           
        }

        zmq_close(socket);
        _exit(0); // Exit child process safely

    } else {
        
        // Enter the game loop
        game_loop(context); // Pass socket to game loop

        // Clean up the ZeroMQ context after the game loop ends
        zmq_ctx_destroy(context);

        wait(NULL); // Wait for child process to terminate
        endwin();  // End the ncurses session and restore terminal settings
        return 0;
    }
}