
#ifndef UNTITLED9_MAIN_H
#define UNTITLED9_MAIN_H

#include <poll.h>
#include <sqlite3.h>

void write_file(int client_socket);
void download_file(int client_socket);
void list_files(int client_socket);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);
sqlite3 * sqlConnect();
int loginForm(int client_socket, int i, int num_clients,sqlite3 *db, struct pollfd *pfds);
int is_username_taken(sqlite3 *db, const char *username);
int receive_username_password(int client_fd, char *username, char *password, sqlite3 *db);
int receive_username_password_login(int client_fd, char *username, char *password, sqlite3 *db);
int username_password_check(sqlite3 *db, const char *username, const char *password);
void handle_registration(int client_fd, sqlite3 *db);
void handle_login(int client_fd, sqlite3 *db);

#endif //UNTITLED9_MAIN_H
