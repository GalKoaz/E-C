
#include <stdlib.h>
#include "server.h"

/**
 * @brief Adds a new file descriptor to the array of pollfd structures.
 *
 * This function adds a new file descriptor to the array of pollfd structures.
 * If the array is already full, it dynamically reallocates memory to accommodate
 * more file descriptors.
 *
 * @param pfds A pointer to the array of pollfd structures.
 * @param newfd The new file descriptor to be added.
 * @param fd_count A pointer to the current count of file descriptors.
 * @param fd_size A pointer to the current size of the file descriptor array.
 */
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {
    // Check if the current array is full
    if (*fd_count == *fd_size) {
        // Double the size of the array
        *fd_size *= 2;
        // Reallocate memory for the larger array
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    // Add the new file descriptor to the array
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*fd_count)++;
}

/**
 * @brief Removes a file descriptor from the array of pollfd structures.
 *
 * This function removes a file descriptor at a specified index from the array
 * of pollfd structures. It does so by replacing the specified element with the
 * last element in the array and then decrementing the file descriptor count.
 *
 * @param pfds The array of pollfd structures.
 * @param i The index of the file descriptor to be removed.
 * @param fd_count A pointer to the current count of file descriptors.
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    // Replace the specified element with the last element
    pfds[i] = pfds[*fd_count - 1];
    // Decrement the file descriptor count
    (*fd_count)--;
}



