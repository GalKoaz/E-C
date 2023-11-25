//
// Created by gal on 10/6/23.
//
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"

#define SIZE 1025
/**
 * @brief Handles the login form for a client.
 *
 * This function receives a request from the client, checks if it is a "register" or "login"
 * request, and then delegates the handling to specific functions accordingly.
 *
 * @param client_socket The socket descriptor for the client.
 * @param i The index of the client in the pollfd array.
 * @param num_clients The number of clients currently connected.
 * @param db The SQLite3 database connection.
 * @param pfds The array of pollfd structures.
 * @return Returns 1 if the operation was successful, 0 otherwise.
 */
int loginForm(int client_socket, int i, int num_clients, sqlite3 *db, struct pollfd *pfds) {
    char request[SIZE];
    ssize_t bytes_received;

    // Initialize the request buffer and ensure null-termination
    memset(request, 0, sizeof(request));
    request[sizeof(request) - 1] = '\0';

    // Receive the client's request
    bytes_received = recv(client_socket, request, sizeof(request) - 1, 0);

    // Check for errors or client disconnection
    if (bytes_received < 0) {
        perror("Error receiving request");
        return 0;
    } else if (bytes_received == 0) {
        printf("Client closed the connection.\n");
        close(client_socket);
        del_from_pfds(pfds, i, &num_clients);
        return 0;
    }

    // Process the client's request
    if (strcmp(request, "register") == 0) {
        handle_registration(client_socket, db);
    } else if (strcmp(request, "login") == 0) {
        handle_login(client_socket, db);
    }

    return 1;
}

/**
 * @brief Handles the menu bar for a client session.
 *
 * This function receives a request from the client and performs different actions based
 * on the received request. Actions include uploading, downloading, listing files, or exiting
 * the session.
 *
 * @param client_socket The socket descriptor for the client.
 * @param i The index of the client in the pollfd array.
 * @param num_clients The number of clients currently connected.
 * @param pfds The array of pollfd structures.
 * @return Returns 0 if the operation was successful, 1 if there was an error or the client requested to exit.
 */
int menuBar(int client_socket, int i, int num_clients, struct pollfd *pfds) {
    char request[SIZE];
    ssize_t bytes_received;

    // Initialize the request buffer
    memset(request, 0, sizeof(request));

    // Receive the client's request
    bytes_received = recv(client_socket, request, sizeof(request), 0);

    // Check for errors or client disconnection
    if (bytes_received < 0) {
        perror("Error receiving request");
        return 1;
    } else if (bytes_received == 0) {
        printf("Client closed the connection.\n");
        close(client_socket);
        del_from_pfds(pfds, i, &num_clients);
        return 1;
    }

    // Ensure null-termination of the received data
    request[bytes_received] = '\0';

    // Process the client's request
    if (strcmp(request, "upload") == 0) {
        write_file(client_socket);
    } else if (strcmp(request, "download") == 0) {
        download_file(client_socket);
    } else if (strcmp(request, "list_files") == 0) {
        list_files(client_socket);
    } else if (strcmp(request, "exit") == 0) {
        printf("Client requested to exit the session.\n");
        close(client_socket);
        del_from_pfds(pfds, i, &num_clients);
        return 1;
    } else {
        printf("Invalid request from client: %s\n", request);
    }

    return 0;
}
