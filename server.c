#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Link the Winsock library

#define BUFFERCAP 1024
#define BOOKCAP 1024
#define LIBRARIESCAP 1024
#define PORT 8080

/* Structure Definitions */

// Date structure
typedef struct {
    int day;
    int month;
    int year;
} Date;

// Author structure
typedef struct {
    char* Name;
    char* LastName;
    Date DateBirth;
} Author;

// Book structure
typedef struct {
    char Name[20];
    char Genre[20];
    Author Auth;
} Book;

// Library structure
typedef struct {
    char* name;
    Book books[BOOKCAP];
} Library;

// Libraries structure
typedef struct {
    Library* libraries;
    int count;
    Library* currentlib;
} Libraries;

/* Global variable for libraries */
Libraries libs;

/* Function Declarations */
void initialize_winsock();
void cleanup_winsock();
void init_Libraries(Libraries* libs);
void handle_client(SOCKET client_socket, Libraries* libs);
void execute_command(const char* command, Libraries* libs, char* response);

/* Initialize Winsock */
void initialize_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Winsock initialized.\n");
}

/* Clean up Winsock */
void cleanup_winsock() {
    WSACleanup();
}

/* Initialize Libraries */
void init_Libraries(Libraries* libs) {
    libs->libraries = malloc(LIBRARIESCAP * sizeof(Library));
    libs->count = 0;
    libs->currentlib = NULL;
    if (libs->libraries == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

/* Handle client requests */
void handle_client(SOCKET client_socket, Libraries* libs) {
    char buffer[BUFFERCAP] = {0};
    char response[BUFFERCAP] = {0};

    while (1) {
        // Receive client input
        int valread = recv(client_socket, buffer, BUFFERCAP, 0);
        if (valread <= 0) {
            printf("Client disconnected or error occurred. Error: %d\n", WSAGetLastError());
            break;
        }

        buffer[valread] = '\0';  // Null-terminate the input

        // Process the command
        execute_command(buffer, libs, response);

        // Send the response back to the client
        send(client_socket, response, strlen(response), 0);
    }

    // Close the client socket after handling the client session
    closesocket(client_socket);
}


/* Execute client command */
void execute_command(const char* command, Libraries* libs, char* response) {
    if (strncmp(command, "show", 4) == 0) {
        if (libs->count == 0) {
            sprintf(response, "No libraries registered.\n");
        } else {
            for (int i = 0; i < libs->count; ++i) {
                if (libs->currentlib != NULL && strcmp(libs->libraries[i].name, libs->currentlib->name) == 0) {
                    sprintf(response + strlen(response), "* %s\n", libs->libraries[i].name);
                } else {
                    sprintf(response + strlen(response), "%s\n", libs->libraries[i].name);
                }
            }
        }
    } else if (strncmp(command, "use", 3) == 0) {
        char lib_name[20];
        sscanf(command, "use %19s", lib_name);
        
        for (int i = 0; i < libs->count; ++i) {
            if (strcmp(libs->libraries[i].name, lib_name) == 0) {
                libs->currentlib = &libs->libraries[i];
                sprintf(response, "Using library: %s\n", lib_name);
                return;
            }
        }

        // If library doesn't exist, create and use it
        if (libs->count < LIBRARIESCAP) {
            Library new_lib;
            new_lib.name = strdup(lib_name);
            libs->libraries[libs->count] = new_lib;
            libs->currentlib = &libs->libraries[libs->count];
            libs->count++;
            sprintf(response, "Created and using library: %s\n", lib_name);
        } else {
            sprintf(response, "Library limit reached.\n");
        }
    } else {
        sprintf(response, "Unknown command: %s\n", command);
    }
}

/* Main function */
int main() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Initialize Winsock
    initialize_winsock();

    // Initialize libraries
    init_Libraries(&libs);

    // Create a server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Bind only to localhost
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }
    printf("Bind successful.\n");

    // Listen for incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }

    printf("Library server is running on port %d...\n", PORT);

    // Accept and handle clients
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            cleanup_winsock();
            return 1;
        }

        printf("Client connected.\n");

        // Handle client request
        handle_client(client_socket, &libs);

        // Close the client socket
        closesocket(client_socket);
    }

    // Cleanup
    closesocket(server_socket);
    cleanup_winsock();

    return 0;
}
