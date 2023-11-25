#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include <stdlib.h>
#include "server.h"

#define SIZE 1024
#define MAX_FILENAME_LENGTH 256


/**
 * @brief Receives a file from the client and saves it on the server.
 *
 * This function receives the filename, file size, and file content from the client,
 * then saves the file on the server. It uses a simple ACK (acknowledgment) mechanism
 * to ensure data integrity during transmission.
 *
 * @param client_socket The socket descriptor for the client.
 */
void write_file(int client_socket) {
    char buffer[SIZE], filename[SIZE], file_size_str[SIZE];
    ssize_t bytes_received;
    off_t total_received = 0;
    int file_size;
    FILE *file;

    // Receive the filename from the client
    bytes_received = recv(client_socket, filename, sizeof(filename), 0);
    if (bytes_received < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    // Receive the file size from the client
    memset(file_size_str, 0, sizeof(file_size_str));
    bytes_received = recv(client_socket, file_size_str, sizeof(file_size_str), 0);
    if (bytes_received < 0) {
        perror("Error receiving file size");
        return;
    }

    // Convert file size string to integer
    file_size = atoi(file_size_str);

    // Open the file for writing in binary mode
    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Send an acknowledgment to the client
    const char *response = "ACK";
    send(client_socket, response, strlen(response), 0);

    // Receive the file content and write it to the file
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

/**
 * @brief Sends a file to the client.
 *
 * This function sends the requested file to the client. It first sends the filename,
 * file size, and then the file content. It uses an ACK mechanism to ensure data integrity
 * during transmission.
 *
 * @param client_socket The socket descriptor for the client.
 */
void download_file(int client_socket) {
    char bufferSize[SIZE], filename[SIZE];
    ssize_t bytes_received;
    FILE *file;

    // Receive the filename from the client
    bytes_received = recv(client_socket, filename, sizeof(filename) - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving filename");
        return;
    }
    filename[bytes_received] = '\0';

    // Open the file for reading in binary mode
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Determine the file size
    memset(bufferSize, 0, sizeof(bufferSize));
    fseek(file, 0L, SEEK_END);
    off_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    snprintf(bufferSize, sizeof(bufferSize), "%ld", file_size);

    // Send the file size to the client
    send(client_socket, bufferSize, strlen(bufferSize), 0);

    // Receive an acknowledgment from the client
    char ack[3];
    recv(client_socket, ack, sizeof(ack), 0);

    size_t bytes_read;
    off_t total_send = 0;

    char buffer[SIZE];

    // Send the file content to the client
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

/**
 * @brief Sends the list of files in the server directory to the client.
 *
 * This function sends the list of files in the server directory to the client.
 * It includes the total length of the file list and uses an ACK mechanism to ensure
 * data integrity during transmission.
 *
 * @param client_socket The socket descriptor for the client.
 */
void list_files(int client_socket) {
    DIR *dir;
    struct dirent *entry;

    // Open the server directory for reading
    dir = opendir("./");
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    char file_list[SIZE] = "";
    size_t file_list_length = 0;

    // Read the file names in the server directory
    while ((entry = readdir(dir)) != NULL) {
        strcat(file_list, entry->d_name);
        strcat(file_list, "\n");
        file_list_length += strlen(entry->d_name) + 1;
    }

    closedir(dir);

    // Convert the file list length to a string
    char length_buffer[16];
    snprintf(length_buffer, sizeof(length_buffer), "%zu", file_list_length);

    // Send the file list length to the client
    send(client_socket, length_buffer, strlen(length_buffer), 0);

    // Receive an acknowledgment from the client
    char ack[3];
    recv(client_socket, ack, sizeof(ack), 0);

    // Send the file list to the client
    send(client_socket, file_list, file_list_length, 0);
    printf("File list sent to the client.\n");
}

