#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include <stdlib.h>
#include "server.h"

#define SIZE 1024
#define MAX_FILENAME_LENGTH 256


void write_file(int client_socket) {
    char buffer[SIZE], filename[SIZE], file_size_str[SIZE];
    ssize_t bytes_received;
    off_t file_size, total_received = 0;
    FILE *file;

    bytes_received = recv(client_socket, filename, sizeof(filename), 0);
    if (bytes_received < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';


    bytes_received = recv(client_socket, file_size_str, sizeof(file_size_str), 0);
    if (bytes_received < 0) {
        perror("Error receiving file size");
        return;
    }


    file_size = atoi(file_size_str);

    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    while (total_received < file_size) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        fwrite(buffer, 1, bytes_received, file);
        total_received += bytes_received;
    }

    fclose(file);
    printf("File '%s' received and saved successfully.\n", filename);
}


void download_file(int client_socket) {
    char bufferSize[SIZE], filename[SIZE];
    ssize_t bytes_received;
    FILE *file;

    bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    memset(bufferSize, 0, sizeof(bufferSize));
    fseek(file, 0L, SEEK_END);
    off_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    snprintf(bufferSize, sizeof(bufferSize), "%ld", file_size);
    send(client_socket, bufferSize, strlen(bufferSize), 0);

    char ack[3];
    recv(client_socket, ack, sizeof(ack), 0);

    size_t bytes_read;
    off_t total_send = 0;

    char buffer[SIZE];

    while (total_send < file_size) {
        bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read <= 0) {
            perror("Error sending file");
            break;
        }
        send(client_socket, buffer, bytes_read, 0);
        total_send += bytes_read;
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
    size_t file_list_length = 0;

    while ((entry = readdir(dir)) != NULL) {

        strcat(file_list, entry->d_name);
        strcat(file_list, "\n");
        file_list_length += strlen(entry->d_name) + 1;
    }

    closedir(dir);

    char length_buffer[16];
    snprintf(length_buffer, sizeof(length_buffer), "%zu", file_list_length);
    send(client_socket, length_buffer, strlen(length_buffer), 0);

    char ack[3];
    recv(client_socket, ack, sizeof(ack), 0);

    send(client_socket, file_list, file_list_length, 0);
    printf("File list sent to the client.\n");
}