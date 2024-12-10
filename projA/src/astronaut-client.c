#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "definitions.h"

// Function to handle user input and send messages to the server
void handle_input(void *socket, char astronaut_id, int num_aliens, int token) {
    int ch = getch(); // Get user input
    Message msg = {0}; // Initialize a Message struct to zero
    msg.astronaut_id = astronaut_id; // Set astronaut ID
    msg.num_aliens = num_aliens; // Set number of aliens
    msg.token = token; // Set token for authentication or identification

    // Loop to handle continuous input until 'q' or 'Q' is pressed
    while ((ch != 'q') && (ch != 'Q')) {      
        
        // Determine the action based on user input
        switch (ch) {
            case KEY_UP:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.x = 0;
                msg.y = -1; // Move up
                break;
            case KEY_DOWN:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.y = 1; // Move down
                msg.x = 0;
                break;
            case KEY_LEFT:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.x = -1; // Move left
                msg.y = 0;
                break;
            case KEY_RIGHT:
                msg.type = ASTRONAUT_MOVEMENT;
                msg.x = 1; // Move right
                msg.y = 0;
                break;
            case ' ':
                msg.type = ASTRONAUT_ZAP; // Fire laser
                break;
            default:
                // Skip to the next input if no valid key was pressed
                ch = getch();
                continue; 
        }
        // Send the message to the server via ZeroMQ socket
        zmq_send(socket, &msg, sizeof(msg), 0);
        /*if (zmq_send(socket, &msg, sizeof(msg), 0) == -1) {
            mvprintw(2, 0, "Failed to send message.");
            refresh();
            break;
        }*/

        // Receive the response from the server
        zmq_recv(socket, &msg, sizeof(msg), 0);
        /*if (zmq_recv(socket, &msg, sizeof(msg), 0) == -1) {
            mvprintw(2, 0, "Failed to receive message.");
            refresh();
            break;
        }*/

        // Display updated score
        mvprintw(1, 0, "Score: %d\n", msg.score);
        refresh(); // Refresh the screen to reflect the changes

        ch = getch(); // Get the next user input
    }

    // Send a disconnect message when 'q' or 'Q' is pressed
    msg.type = ASTRONAUT_DISCONNECT;
    if (zmq_send(socket, &msg, sizeof(msg), 0) == -1) {
        mvprintw(2, 0, "Failed to send disconnect message.");
        refresh();
    }
}

int main() {
    // Initialize the ncurses library for terminal handling
    if (initscr() == NULL) {
        perror("Error initializing ncurses");
        return 1;
    }
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // Create a ZeroMQ context for communication
    void *context = zmq_ctx_new(); 
    if (context == NULL) {
        perror("Error creating ZeroMQ context");
        endwin();
        return 1;
    }
    void *socket = zmq_socket(context, ZMQ_REQ); // Create a request (REQ) socket
    if (!socket) {
        perror("Failed to create REQ socket");
        exit(1);
    }
    
    if (zmq_connect(socket, SOCKET_ADDRESS_CLIENT) != 0) {  // Connect to the server address    
        perror("Failed to connect SUB socket");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return 1; 
    }
    
    // Initialize the connect message
    Message msg = {0};
    msg.type = ASTRONAUT_CONNECT;
    msg.num_aliens = MAX_ALIENS;

    if (zmq_send(socket, &msg, sizeof(msg), 0) == -1) { // Send the connect message
        perror("Failed to send connect message");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        endwin();
        exit(1);
    }
    
    if (zmq_recv(socket, &msg, sizeof(msg), 0) == -1) { // Receive the server's response
        perror("Failed to receive message");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        endwin();
        exit(1);
    }

    // Check if the connection was refused due to too many players
    if (msg.type == REFUSED_CONNECTION){
        mvprintw(0, 0, "Maximum number of players reached."); // Display an error message
        refresh();
        usleep(5000000); // Wait for 5 seconds before exiting
        zmq_close(socket);
        zmq_ctx_destroy(context);
        endwin();
        exit(1);
    }

    // Connection successful: display astronaut ID and proceed
    char astronaut_id = msg.astronaut_id;
    int num_aliens = msg.num_aliens;
    int token = msg.token;
    mvprintw(0, 0, "Connected as astronaut %c\n", astronaut_id);
    refresh();

    // Call the function to handle user input
    handle_input(socket, astronaut_id, num_aliens, token);

    // Cleanup resources before exiting
    zmq_close(socket); // Close the ZeroMQ socket
    zmq_ctx_destroy(context); // Destroy the ZeroMQ context
    endwin(); // End ncurses mode
    return 0;
}