#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "ws2_32.lib")   // Link the Winsock library
#pragma comment(lib, "bcrypt.lib")   // Link the bcrypt library

#define BUFFERCAP 1024
#define BOOKCAP 1024
#define LIBRARIESCAP 1024
#define PORT 8080
#define HASH_SIZE 32  // SHA-256 produces 32 bytes

/* -- Structure Definitions -- */

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

/* -- Global Libraries Variable -- */
Libraries libs;

/* -- Function Prototypes -- */
void initialize_winsock();
SOCKET create_server_socket();
void bind_server_socket(SOCKET server_socket, struct sockaddr_in* server_addr);
DWORD WINAPI handle_client(void* socket_desc);
int login_user(SOCKET client_socket);
int register_user(SOCKET client_socket);
void execute_command(const char* command, Libraries* libs, char* response);
void init_libraries(Libraries* libs);
void cleanup_winsock();
void hash_password(const char* password, char* hashed_password);
int verify_password(const char* password, const char* hashed_password);
int find_user(const char* username, char* hashed_password);
void save_user(const char* username, const char* hashed_password);

/* -- Function Implementations -- */

/**
 * Initializes Winsock on the server side.
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
 * Creates and returns a server socket.
 * The socket is created with the TCP protocol (SOCK_STREAM).
 */
SOCKET create_server_socket() {
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

/**
 * Binds the server socket to the specified IP address and port.
 * 
 * @param server_socket The server socket to bind.
 * @param server_addr The server address structure containing IP and port information.
 */
void bind_server_socket(SOCKET server_socket, struct sockaddr_in* server_addr) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");  // Binding to localhost
    server_addr->sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)server_addr, sizeof(*server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }
    printf("Bind successful.\n");
}

/**
 * Initializes the Libraries structure, which will hold library information.
 * This must be called before any library operations.
 * 
 * @param libs Pointer to the Libraries structure to initialize.
 */
void init_libraries(Libraries* libs) {
    libs->libraries = malloc(LIBRARIESCAP * sizeof(Library));
    libs->count = 0;
    libs->currentlib = NULL;
    if (libs->libraries == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Handles communication with a single client in a separate thread.
 * This function requires the client to login or register before proceeding.
 * 
 * @param socket_desc Pointer to the client socket descriptor.
 */
DWORD WINAPI handle_client(void* socket_desc) {
    SOCKET client_socket = *(SOCKET*)socket_desc;
    free(socket_desc);  // Free the dynamically allocated socket descriptor

    // Login or Register before proceeding
    int logged_in = 0;
    while (!logged_in) {
        char choice[BUFFERCAP] = {0};
        recv(client_socket, choice, BUFFERCAP, 0);

        if (strcmp(choice, "login") == 0) {
            logged_in = login_user(client_socket);
        } else if (strcmp(choice, "register") == 0) {
            logged_in = register_user(client_socket);
        } else {
            const char* invalid_response = "Invalid choice. Type 'login' or 'register'.\n";
            send(client_socket, invalid_response, strlen(invalid_response), 0);
        }
    }

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
        execute_command(buffer, &libs, response);

        // Send the response back to the client
        send(client_socket, response, strlen(response), 0);
    }

    // Close the client socket after handling the client session
    closesocket(client_socket);
    return 0;
}

/**
 * Login function that validates the user's credentials.
 * 
 * @param client_socket The socket for the connected client.
 * @return 1 if the login is successful, 0 otherwise.
 */
int login_user(SOCKET client_socket) {
    char username[BUFFERCAP] = {0};
    char password[BUFFERCAP] = {0};
    char hashed_password[HASH_SIZE] = {0};
    char stored_hashed_password[HASH_SIZE] = {0};

    // Prompt for username and password
    const char* user_prompt = "Username: ";
    send(client_socket, user_prompt, strlen(user_prompt), 0);
    recv(client_socket, username, BUFFERCAP, 0);

    const char* pass_prompt = "Password: ";
    send(client_socket, pass_prompt, strlen(pass_prompt), 0);
    recv(client_socket, password, BUFFERCAP, 0);

    // Check if the user exists and validate the password
    if (find_user(username, stored_hashed_password)) {
        hash_password(password, hashed_password);
        if (verify_password(hashed_password, stored_hashed_password)) {
            const char* success_msg = "Login successful!\n";
            send(client_socket, success_msg, strlen(success_msg), 0);
            return 1;
        } else {
            const char* fail_msg = "Invalid password.\n";
            send(client_socket, fail_msg, strlen(fail_msg), 0);
        }
    } else {
        const char* fail_msg = "User not found.\n";
        send(client_socket, fail_msg, strlen(fail_msg), 0);
    }

    return 0;
}

/**
 * Registration function that registers a new user with a username and password.
 * 
 * @param client_socket The socket for the connected client.
 * @return 1 if the registration is successful, 0 otherwise.
 */
int register_user(SOCKET client_socket) {
    char username[BUFFERCAP] = {0};
    char password[BUFFERCAP] = {0};
    char hashed_password[HASH_SIZE] = {0};

    // Prompt for username and password
    const char* user_prompt = "Choose a username: ";
    send(client_socket, user_prompt, strlen(user_prompt), 0);
    recv(client_socket, username, BUFFERCAP, 0);

    const char* pass_prompt = "Choose a password: ";
    send(client_socket, pass_prompt, strlen(pass_prompt), 0);
    recv(client_socket, password, BUFFERCAP, 0);

    // Hash the password
    hash_password(password, hashed_password);

    // Save the new user
    save_user(username, hashed_password);

    const char* success_msg = "Registration successful!\n";
    send(client_socket, success_msg, strlen(success_msg), 0);
    return 1;
}

/**
 * Hashes the password using bcrypt SHA-256.
 * 
 * @param password The plaintext password.
 * @param hashed_password The output buffer for the hashed password.
 */
void hash_password(const char* password, char* hashed_password) {
    BCRYPT_ALG_HANDLE hAlgorithm;
    BCRYPT_HASH_HANDLE hHash;
    DWORD cbHashObject, cbData, cbHash;

    // Open algorithm provider
    BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0);

    // Get the length of the hash object
    BCryptGetProperty(hAlgorithm, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbHashObject, sizeof(DWORD), &cbData, 0);

    // Allocate the hash object
    PUCHAR pbHashObject = (PUCHAR)malloc(cbHashObject);
    BCryptCreateHash(hAlgorithm, &hHash, pbHashObject, cbHashObject, NULL, 0, 0);

    // Hash the password
    BCryptHashData(hHash, (PUCHAR)password, (ULONG)strlen(password), 0);
    BCryptFinishHash(hHash, (PUCHAR)hashed_password, HASH_SIZE, 0);

    // Cleanup
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlgorithm, 0);
    free(pbHashObject);
}

/**
 * Verifies if the provided password matches the stored hashed password.
 * 
 * @param password The hashed password from the user input.
 * @param hashed_password The stored hashed password.
 * @return 1 if the passwords match, 0 otherwise.
 */
int verify_password(const char* password, const char* hashed_password) {
    return memcmp(password, hashed_password, HASH_SIZE) == 0;
}

/**
 * Finds a user by username in the `users.txt` file and retrieves the hashed password.
 * 
 * @param username The username to search for.
 * @param hashed_password The buffer to store the retrieved hashed password.
 * @return 1 if the user is found, 0 otherwise.
 */
int find_user(const char* username, char* hashed_password) {
    FILE* file = fopen("users.txt", "r");
    if (!file) return 0;

    char file_username[BUFFERCAP];
    char file_hashed_password[HASH_SIZE];

    while (fscanf(file, "%s %s", file_username, file_hashed_password) != EOF) {
        if (strcmp(file_username, username) == 0) {
            strcpy(hashed_password, file_hashed_password);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

/**
 * Saves a new user's credentials to the `users.txt` file.
 * 
 * @param username The username to save.
 * @param hashed_password The hashed password to save.
 */
void save_user(const char* username, const char* hashed_password) {
    FILE* file = fopen("users.txt", "a");
    if (file) {
        fprintf(file, "%s %s\n", username, hashed_password);
        fclose(file);
    }
}

/**
 * Cleans up Winsock and releases any resources allocated during the server's operation.
 */
void cleanup_winsock() {
    WSACleanup();
}

/* -- Main Function -- */

int main() {
    SOCKET server_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Initialize Winsock
    initialize_winsock();

    // Initialize libraries
    init_libraries(&libs);

    // Create server socket
    server_socket = create_server_socket();

    // Bind the server socket to the localhost IP and port
    bind_server_socket(server_socket, &server_addr);

    // Listen for incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        cleanup_winsock();
        return 1;
    }

    printf("Library server is running on port %d...\n", PORT);

    // Accept and handle clients
    while (1) {
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected.\n");

        // Create a new thread for the client
        new_sock = malloc(sizeof(SOCKET));
        *new_sock = client_socket;
        CreateThread(NULL, 0, handle_client, (void*)new_sock, 0, NULL);
    }

    // Cleanup Winsock
    cleanup_winsock();

    return 0;
}
