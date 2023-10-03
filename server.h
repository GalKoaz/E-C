
#ifndef UNTITLED9_MAIN_H
#define UNTITLED9_MAIN_H

#include <poll.h>
void write_file(int client_socket);
void download_file(int client_socket);
void list_files(int client_socket);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

#endif //UNTITLED9_MAIN_H
