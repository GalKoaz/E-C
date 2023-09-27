#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"

#define PORT 13006
#define SA struct sockaddr
#define SIZE 1024


int main() {
    int sockfd, connfd;
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

    while (1) {
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0) {
            perror("server accept failed...");
            continue;
        } else {
            printf("server accepted the client...\n");
        }

        while (1) {
            char request[SIZE];
            ssize_t bytes_received;
            memset(request, 0, sizeof(request));

            bytes_received = recv(connfd, request, sizeof(request), 0);
            if (bytes_received < 0) {
                perror("Error receiving request");
                break;
            } else if (bytes_received == 0) {
                printf("Client closed the connection.\n");
                break;
            }

            request[bytes_received] = '\0';

            if (strcmp(request, "upload") == 0) {
                write_file(connfd);
            } else if (strcmp(request, "download") == 0) {
                download_file(connfd);
            } else if (strcmp(request, "list_files") == 0) {
                list_files(connfd);
            } else if (strcmp(request, "exit") == 0) {
                printf("Client requested to exit the session.\n");
                break;
            } else {
                printf("Invalid request from client: %s\n", request);
            }
        }

        close(connfd);
    }
}
