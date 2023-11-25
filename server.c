#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include "server.h"

#define PORT 13040
#define SA struct sockaddr


/**
 * @brief Main function for the server application.
 *
 * This function sets up a TCP server, listens for incoming connections,
 * and handles multiple clients using the poll mechanism. It creates a
 * SQLite3 database connection, initializes the polling structure, and
 * enters a loop to wait for client connections. Once a client connects,
 * it adds the client to the polling structure and proceeds to handle
 * login and menu operations.
 *
 * @return Returns 0 on successful execution.
 */
int main() {
    int sockfd, newfd, MAX_CLIENTS = 5, num_clients = 0;
    unsigned len;
    struct sockaddr_in servaddr, cli;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    // Initialize the server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    // Start listening for incoming connections
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    // Establish an SQLite3 database connection
    sqlite3 * db = sqlConnect();

    // Set up the initial polling structure
    len = sizeof(cli);
    struct pollfd *pfds = malloc(sizeof *pfds * MAX_CLIENTS);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    num_clients = 1;

    // Main server loop
    while (1) {
        int ret = poll(pfds, num_clients, -1);
        if (ret == -1) {
            perror("poll");
            sqlite3_close(db);
            exit(1);
        }

        // Check each descriptor in the polling structure
        for (int i = 0; i < num_clients; ++i) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == sockfd) {
                    // Accept a new client connection
                    newfd = accept(sockfd, (SA *) &cli, &len);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        // Add the new client to the polling structure
                        add_to_pfds(&pfds, newfd, &num_clients, &MAX_CLIENTS);
                        printf("Client connected from %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

                        // Handle login for the new client
                        loginForm(newfd, i, num_clients, db, pfds);
                    }
                } else {
                    // Handle menu operations for existing clients
                    if (!menuBar(pfds[i].fd, i, num_clients, pfds)) {
                        break;
                    }
                }
            }
        }
    }
}
