#include <stdio.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <string.h>

#include "server.h"

#define SIZE 1025

sqlite3 * sqlConnect(){
    sqlite3 *db = NULL;
    int rc = sqlite3_open("data.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    } else {
        printf("SQL Connected!\n");
    }
    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS users ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "username TEXT NOT NULL,"
                                   "password TEXT NOT NULL);";
    rc = sqlite3_exec(db, create_table_sql, 0, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error during table creation: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    return db;
}

int receive_username_password(int client_fd, char *username, char *password, sqlite3 *db) {
    ssize_t bytes_received;

    // Receive username
    bytes_received = recv(client_fd, username, SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving username");
        return -1;
    }
    username[bytes_received] = '\0';

    int flag = is_username_taken(db, username);
    while (flag){
        const char *response = "USERNAME_TAKEN";
        send(client_fd, response, strlen(response), 0);
        bytes_received = recv(client_fd, username, SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving username");
            return -1;
        }
        username[bytes_received] = '\0';
        flag = is_username_taken(db, username);
    }

    // Send acknowledgment
    const char *ack = "ACK";
    if (send(client_fd, ack, strlen(ack), 0) < 0) {
        perror("Error sending acknowledgment");
        return -1;
    }

    // Receive password
    bytes_received = recv(client_fd, password, SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving password");
        return -1;
    }
    password[bytes_received] = '\0';

    // Insert username and password into the database
    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    fprintf(stdout, "User '%s' inserted successfully\n",username);
    sqlite3_finalize(stmt);
    return 0;
}


int is_username_taken(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT username FROM users WHERE username = ?";
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    int result = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Username is already in the database
        result = 1;
    }

    sqlite3_finalize(stmt);
    return result;
}

int username_password_check(sqlite3 *db, const char *username, const char *password) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT username FROM users WHERE username = ? AND password = ?";
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int result = 1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Username and password match in the database
        result = 0;
    }

    sqlite3_finalize(stmt);
    return result;
}


int receive_username_password_login(int client_fd, char *username, char *password, sqlite3 *db) {
    ssize_t bytes_received;
    int flag = 1;

    while(flag){
        // Receive username
        bytes_received = recv(client_fd, username, SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving username");
            return -1;
        }
        username[bytes_received] = '\0';

        // Send acknowledgment
        const char *ack = "ACK";
        if (send(client_fd, ack, strlen(ack), 0) < 0) {
            perror("Error sending acknowledgment");
            return -1;
        }

        // Receive password
        bytes_received = recv(client_fd, password, SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving password");
            return -1;
        }
        password[bytes_received] = '\0';

        flag = username_password_check(db, username,password);
        if(flag){
            const char *response = "USERNAME_PASSWORD";
            if (send(client_fd, response, strlen(response), 0) < 0) {
                perror("Error sending acknowledgment");
                return -1;
            }
        }
    }
    return 0;
}


void handle_login(int client_fd, sqlite3 *db){
    char username[SIZE];
    char password[SIZE];

    if (receive_username_password_login(client_fd, username, password, db) == 0) {
        const char *response = "OK";
        send(client_fd, response, strlen(response), 0);
    } else {
        const char *response = "BAD";
        send(client_fd, response, strlen(response), 0);
    }
}

// Function to handle the registration process
void handle_registration(int client_fd, sqlite3 *db) {
    char username[SIZE];
    char password[SIZE];

    if (receive_username_password(client_fd, username, password, db) == 0) {
        const char *response = "OK";
        send(client_fd, response, strlen(response), 0);
    } else {
        const char *response = "BAD";
        send(client_fd, response, strlen(response), 0);
    }
}
