#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include <arpa/inet.h>
#include <poll.h>

#define PORT 13016
#define SA struct sockaddr
#define SIZE 1024


int main() {
    int sockfd = 0, newfd = 0, MAX_CLIENTS = 5;
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
    } else
        printf("Server listening..\n");
    len = sizeof(cli);

    int num_clients = 0;

    struct pollfd *pfds = malloc(sizeof *pfds * MAX_CLIENTS);


    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    num_clients = 1;


    while (1) {
        int ret = poll(pfds, num_clients, -1);
        if (ret == -1) {
            perror("poll");
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
                    }
                } else {
                    char request[SIZE];
                    ssize_t bytes_received;
                    memset(request, 0, sizeof(request));

                    bytes_received = recv(pfds[i].fd, request, sizeof(request), 0);
                    if (bytes_received < 0) {
                        perror("Error receiving request");
                        break;
                    } else if (bytes_received == 0) {
                        printf("Client closed the connection.\n");
                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &num_clients);
                        break;
                    }

                    request[bytes_received] = '\0';

                    if (strcmp(request, "upload") == 0) {
                        write_file(pfds[i].fd);
                    } else if (strcmp(request, "download") == 0) {
                        download_file(pfds[i].fd);
                    } else if (strcmp(request, "list_files") == 0) {
                        list_files(pfds[i].fd);
                    } else if (strcmp(request, "exit") == 0) {
                        printf("Client requested to exit the session.\n");
                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &num_clients);
                        break;
                    } else {
                        printf("Invalid request from client: %s\n", request);
                    }
                }
            }
        }
    }
}