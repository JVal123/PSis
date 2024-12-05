#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include <string.h>
#include "definitions.h"

void handle_input(void *socket, char astronaut_id) {
    int ch;
    Message msg = {0, 0, 0, 0, 0};
    msg.astronaut_id = astronaut_id;

    while ((ch = getch()) != 'q') {
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
                continue;
        }

        zmq_send(socket, &msg, sizeof(msg), 0);
        zmq_recv(socket, &msg, sizeof(msg), 0);

        // Display updated score
        mvprintw(1, 0, "Score: %d", msg.score);
        refresh();
    }

    // Send disconnect message
    msg.type = ASTRONAUT_DISCONNECT;
    zmq_send(socket, &msg, sizeof(msg), 0);
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // Connect to server
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, SOCKET_ADDRESS_CLIENT);

    // Send connect message
    Message msg = {ASTRONAUT_CONNECT, 0, 0, 0, 0};
    zmq_send(socket, &msg, sizeof(msg), 0);
    zmq_recv(socket, &msg, sizeof(msg), 0);

    char astronaut_id = msg.astronaut_id;
    int score = msg.score;
    mvprintw(0, 0, "Connected as astronaut %c with score %i\n", astronaut_id, score);
    refresh();

    // Handle user input
    handle_input(socket, astronaut_id);

    // Cleanup
    zmq_close(socket);
    zmq_ctx_destroy(context);
    endwin();
    return 0;
}