#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>

#include "server.h"

#define PORT 8080
#define SIZE 1024


void write_file(int client_socket) {
    char buffer[SIZE];
    char filename[SIZE];
    ssize_t bytes_received;
    FILE *file;

    bytes_received = recv(client_socket, filename, sizeof(filename), 0);
    if (bytes_received < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    off_t file_size;
    bytes_received = recv(client_socket, &file_size, sizeof(off_t), 0);
    if (bytes_received < 0) {
        perror("Error receiving file size");
        return;
    }

    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    off_t total_received = 0;
    while (total_received < file_size) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        fwrite(buffer, 1, bytes_received, file);
        total_received += bytes_received;

        if (strstr(buffer, "EOF") != NULL) {
            break;
        }
    }

    fclose(file);
    printf("File '%s' received and saved successfully.\n", filename);
}

void download_file(int client_socket) {
    char filename[SIZE];
    ssize_t bytes_sent;
    FILE *file;

    bytes_sent = recv(client_socket, filename, sizeof(filename), 0);
    if (bytes_sent < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_sent] = '\0';

    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char buffer[SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Error sending file");
            break;
        }
    }

    fclose(file);
    printf("File '%s' sent successfully.\n", filename);
}

void list_files(int client_socket) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir("./");
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    char file_list[SIZE] = "";

    while ((entry = readdir(dir)) != NULL) {
        strcat(file_list, entry->d_name);
        strcat(file_list, "\n");
    }

    closedir(dir);

    send(client_socket, file_list, strlen(file_list), 0);
    printf("File list sent to the client.\n");
    return;
}