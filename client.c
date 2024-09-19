#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Link the Winsock library

#define PORT 8080
#define BUFFERCAP 1024

/* -- Function Prototypes -- */
void initialize_winsock();
SOCKET create_client_socket();
void connect_to_server(SOCKET sock, struct sockaddr_in* serv_addr);
void login_or_register(SOCKET sock);
void interact_with_server(SOCKET sock);
void cleanup_winsock();

/* -- Function Implementations -- */

/**
 * Initializes Winsock on the client side.
 * This must be called before any socket operations.
 */
void initialize_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Winsock initialized.\n");
}

/**
 * Creates and returns a client socket.
 * The socket is created with the TCP protocol (SOCK_STREAM).
 */
SOCKET create_client_socket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }
    return sock;
}

/**
 * Connects the client socket to the server at the specified address.
 * 
 * @param sock The client socket to connect.
 * @param serv_addr The server address structure containing IP and port information.
 */
void connect_to_server(SOCKET sock, struct sockaddr_in* serv_addr) {
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);
    serv_addr->sin_addr.s_addr = inet_addr("127.0.0.1");  // Connecting to localhost

    // Connect to the server
    if (connect(sock, (struct sockaddr*)serv_addr, sizeof(*serv_addr)) < 0) {
        printf("Connection to server failed with error code: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }
    printf("Connected to server.\n");
}

/**
 * Allows the client to login or register with the server.
 * 
 * @param sock The connected client socket.
 */
void login_or_register(SOCKET sock) {
    char buffer[BUFFERCAP] = {0};
    char choice[BUFFERCAP] = {0};

    while (1) {
        printf("Do you want to 'login' or 'register'?: ");
        fgets(choice, BUFFERCAP, stdin);
        choice[strcspn(choice, "\n")] = 0;  // Remove newline

        // Send choice to server
        send(sock, choice, strlen(choice), 0);

        // Receive response from server
        recv(sock, buffer, BUFFERCAP, 0);
        printf("%s", buffer);

        // Handle login/register process based on choice
        if (strcmp(choice, "login") == 0 || strcmp(choice, "register") == 0) {
            char username[BUFFERCAP], password[BUFFERCAP];

            // Get username and password
            recv(sock, buffer, BUFFERCAP, 0);
            printf("%s", buffer);  // Username prompt
            fgets(username, BUFFERCAP, stdin);
            username[strcspn(username, "\n")] = 0;  // Remove newline
            send(sock, username, strlen(username), 0);

            recv(sock, buffer, BUFFERCAP, 0);
            printf("%s", buffer);  // Password prompt
            fgets(password, BUFFERCAP, stdin);
            password[strcspn(password, "\n")] = 0;  // Remove newline
            send(sock, password, strlen(password), 0);

            // Receive success or failure message
            recv(sock, buffer, BUFFERCAP, 0);
            printf("%s", buffer);

            if (strstr(buffer, "successful")) {
                break;  // Login or registration successful
            }
        } else {
            printf("Invalid choice. Type 'login' or 'register'.\n");
        }
    }
}

/**
 * Interacts with the server by sending commands and receiving responses.
 * This runs in a loop, allowing the user to send multiple commands to the server.
 * 
 * @param sock The connected client socket.
 */
void interact_with_server(SOCKET sock) {
    char buffer[BUFFERCAP] = {0};
    char command[BUFFERCAP] = {0};

    while (1) {
        // Prompt for command
        printf("Enter command (use <library> / show / exit): ");
        fgets(command, BUFFERCAP, stdin);
        command[strcspn(command, "\n")] = 0;  // Remove newline

        // Exit if the user types 'exit'
        if (strcmp(command, "exit") == 0) {
            break;
        }

        // Send command to server
        send(sock, command, strlen(command), 0);

        // Receive response from server
        int valread = recv(sock, buffer, BUFFERCAP, 0);
        if (valread <= 0) {
            printf("Error reading from server: %d\n", WSAGetLastError());
            break;
        }
        buffer[valread] = '\0';  // Null-terminate response
        printf("%s\n", buffer);
    }

    // Close the socket once done
    closesocket(sock);
}

/**
 * Cleans up Winsock and releases any resources allocated during the client's operation.
 */
void cleanup_winsock() {
    WSACleanup();
}

/* -- Main Function -- */

int main() {
    SOCKET sock;
    struct sockaddr_in serv_addr;

    // Initialize Winsock
    initialize_winsock();

    // Create client socket
    sock = create_client_socket();

    // Connect to the server
    connect_to_server(sock, &serv_addr);

    // Login or register
    login_or_register(sock);

    // Interact with the server
    interact_with_server(sock);

    // Cleanup Winsock
    cleanup_winsock();

    return 0;
}
