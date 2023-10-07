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


int main() {
    int sockfd, newfd, MAX_CLIENTS = 5, num_clients = 0;
    unsigned len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    // Sql connection:
    sqlite3 * db = sqlConnect();

    len = sizeof(cli);

    struct pollfd *pfds = malloc(sizeof *pfds * MAX_CLIENTS);

    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    num_clients = 1;


    while (1) {
        int ret = poll(pfds, num_clients, -1);
        if (ret == -1) {
            perror("poll");
            sqlite3_close(db);
            exit(1);
        }
        for (int i = 0; i < num_clients; ++i) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == sockfd) {
                    newfd = accept(sockfd, (SA *) &cli, &len);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &num_clients, &MAX_CLIENTS);
                        printf("Client connected from %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
                        loginForm(newfd, i, num_clients, db, pfds);
                    }
                } else {
                    if(!menuBar(pfds[i].fd, i, num_clients, pfds)){
                        break;
                    }
                }
            }
        }
    }
    sqlite3_close(db);
}