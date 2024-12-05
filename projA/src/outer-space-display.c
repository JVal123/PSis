#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <ncurses.h>
#include "definitions.h"

void update_display(Message *state) {
    clear();
    mvprintw(0, 0, "Outer Space Display");

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            char display_char = ' ';
            if (state->x == i && state->y == j) {
                display_char = '*'; // Alien or zap effect
            }
            mvaddch(i + 1, j, display_char);
        }
    }

    refresh();
}

int main() {
    initscr();
    noecho();
    cbreak();

    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(socket, SOCKET_ADDRESS_SERVER);
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

    Message msg;

    while (1) {
        zmq_recv(socket, &msg, sizeof(msg), 0);
        update_display(&msg);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    endwin();
    return 0;
}