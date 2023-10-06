//
// Created by gal on 10/6/23.
//
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"

#define SIZE 1025

int loginForm(int client_socket, int i, int num_clients,sqlite3 *db, struct pollfd *pfds){
    char request[SIZE];
    ssize_t bytes_received;
    memset(request, 0, sizeof(request));

    bytes_received = recv(client_socket, request, sizeof(request), 0);
    if (bytes_received < 0) {
        perror("Error receiving request");
        return 0;
    } else if (bytes_received == 0) {
        printf("Client closed the connection.\n");
        close(client_socket);
        del_from_pfds(pfds, i, &num_clients);
        return 0;
    }
    request[bytes_received] = '\0';
    if (strcmp(request, "register") == 0) {
        handle_registration(client_socket, db);
    }else if (strcmp(request, "login") == 0) {
        handle_login(client_socket, db);
    }
    return 1;
}
