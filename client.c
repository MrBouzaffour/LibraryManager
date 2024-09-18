#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Link the Winsock library

#define PORT 8080
#define BUFFERCAP 1024

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

/* Main function for client */
int main() {
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFERCAP] = {0};
    char command[BUFFERCAP] = {0};

    // Initialize Winsock
    initialize_winsock();

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Directly set localhost IP

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed with error code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }

    // Client input loop
    while (1) {
        printf("Enter command (use <library> / show / exit): ");
        fgets(command, BUFFERCAP, stdin);
        command[strcspn(command, "\n")] = 0;  // Remove newline

        // Exit client
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
        buffer[valread] = '\0';
        printf("%s\n", buffer);
    }

    // Close the socket
    closesocket(sock);
    cleanup_winsock();
    return 0;
}
