#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include <arpa/inet.h>
#include <poll.h>

#define PORT 13008
#define SA struct sockaddr
#define SIZE 1024
#define MAX_CLIENTS 5


int main() {
    int sockfd, connfd[MAX_CLIENTS];
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

    struct pollfd poll_fds[MAX_CLIENTS + 1];
    poll_fds[0].fd = sockfd;
    poll_fds[0].events = POLLIN;

    int num_clients = 0;

    while (1) {
        int ret = poll(poll_fds, num_clients + 1, -1);
        if (ret == -1) {
            perror("Error in poll!");
        }

        if (poll_fds[0].revents & POLLIN) {
            connfd[num_clients] = accept(sockfd, (SA *) &cli, &len);
            if (connfd[num_clients] == -1) {
                perror("Error accepting connection");
            } else {
                printf("Client connected from %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
                poll_fds[num_clients + 1].fd = connfd[num_clients];
                poll_fds[num_clients + 1].events = POLLIN;
                num_clients++;
            }
        }

        for (int i = 0; i < num_clients; i++) {
            if (poll_fds[i + 1].revents & POLLIN) {
                char request[SIZE];
                ssize_t bytes_received;
                memset(request, 0, sizeof(request));

                bytes_received = recv(poll_fds[i + 1].fd, request, sizeof(request), 0);
                if (bytes_received < 0) {
                    perror("Error receiving request");
                    break;
                } else if (bytes_received == 0) {
                    printf("Client closed the connection.\n");
                    break;
                }

                request[bytes_received] = '\0';

                if (strcmp(request, "upload") == 0) {
                    write_file(poll_fds[i + 1].fd);
                } else if (strcmp(request, "download") == 0) {
                    download_file(poll_fds[i + 1].fd);
                } else if (strcmp(request, "list_files") == 0) {
                    list_files(poll_fds[i + 1].fd);
                } else if (strcmp(request, "exit") == 0) {
                    printf("Client requested to exit the session.\n");
                    break;
                } else {
                    printf("Invalid request from client: %s\n", request);
                }
            }
        }
    }
    for (int i = 0; i < num_clients; i++) {
        close(connfd[i]);
    }
}
