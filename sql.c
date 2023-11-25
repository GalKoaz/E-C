#include <stdio.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <string.h>

#include "server.h"

#define SIZE 1025

/**
 * @brief Establishes a connection to the SQLite3 database.
 *
 * This function opens a connection to the SQLite3 database named "data.db"
 * and creates a 'users' table if it doesn't exist. It returns the SQLite3
 * database handle for further interactions.
 *
 * @return Returns the SQLite3 database handle on success, NULL on failure.
 */
sqlite3 *sqlConnect() {
    sqlite3 *db = NULL;
    int rc = sqlite3_open("data.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    } else {
        printf("SQL Connected!\n");
    }

    // Create 'users' table if not exists
    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS users ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "username TEXT NOT NULL,"
                                   "password TEXT NOT NULL);";
    rc = sqlite3_exec(db, create_table_sql, 0, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error during table creation: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

/**
 * @brief Receives a username and password from the client for registration.
 *
 * This function receives a username and password from the client, checks if
 * the username is already taken, and inserts the new user into the 'users'
 * table in the SQLite3 database.
 *
 * @param client_fd The client socket descriptor.
 * @param username Buffer to store the received username.
 * @param password Buffer to store the received password.
 * @param db The SQLite3 database handle.
 * @return Returns 0 on successful registration, -1 on failure.
 */
int receive_username_password(int client_fd, char *username, char *password, sqlite3 *db) {
    ssize_t bytes_received;

    // Receive username
    bytes_received = recv(client_fd, username, SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving username");
        return -1;
    }
    username[bytes_received] = '\0';

    // Check if username is taken
    int flag = is_username_taken(db, username);
    while (flag) {
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

    fprintf(stdout, "User '%s' inserted successfully\n", username);
    sqlite3_finalize(stmt);
    return 0;
}
/**
 * @brief Checks if a given username is already taken in the database.
 *
 * This function queries the 'users' table in the SQLite3 database to check
 * if the provided username is already present.
 *
 * @param db The SQLite3 database handle.
 * @param username The username to check.
 * @return Returns 1 if the username is taken, 0 if it's available, -1 on error.
 */
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

/**
 * @brief Checks if the provided username and password match in the database.
 *
 * This function queries the 'users' table in the SQLite3 database to check
 * if the provided username and password match an existing user.
 *
 * @param db The SQLite3 database handle.
 * @param username The username to check.
 * @param password The password to check.
 * @return Returns 0 if the username and password match, 1 if they don't, -1 on error.
 */
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
/**
 * @brief Receives a username and password from the client for login.
 *
 * This function repeatedly receives a username and password from the client,
 * checks if they match an existing user in the database, and sends an ACK
 * or USERNAME_PASSWORD response accordingly.
 *
 * @param client_fd The client socket descriptor.
 * @param username Buffer to store the received username.
 * @param password Buffer to store the received password.
 * @param db The SQLite3 database handle.
 * @return Returns 0 on successful login, -1 on failure.
 */
int receive_username_password_login(int client_fd, char *username, char *password, sqlite3 *db) {
    ssize_t bytes_received;
    int flag = 1;

    while (flag) {
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

        // Check if username and password match
        flag = username_password_check(db, username, password);
        if (flag) {
            const char *response = "USERNAME_PASSWORD";
            if (send(client_fd, response, strlen(response), 0) < 0) {
                perror("Error sending acknowledgment");
                return -1;
            }
        }
    }
    return 0;
}
/**
 * @brief Handles the login process for a client.
 *
 * This function receives a username and password from the client for login
 * and sends an acknowledgment or BAD response based on the success of the
 * login process.
 *
 * @param client_fd The client socket descriptor.
 * @param db The SQLite3 database handle.
 */
void handle_login(int client_fd, sqlite3 *db) {
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

/**
 * @brief Handles the registration process for a client.
 *
 * This function calls `receive_username_password` to receive the
 * username and password from the client during registration.
 **/

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
