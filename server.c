#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "stdio.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#include <unistd.h>


#define CLIENT_PORT 21
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

void send_message(const int fd, char buffer[], const int buffer_len) {
    const ssize_t bytes_sent = send(fd, buffer, buffer_len, 0);
    if (bytes_sent == -1) {
        perror("The send function failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void handle_list(char *response) {
    strcpy(response,"Hello world");
}



void handle_command(const int fd, char command[]) {
    char response_buffer[BUFFER_SIZE];

    if (strcmp(command, "LIST") == 0) handle_list(response_buffer);

    send_message(fd, response_buffer, strlen(response_buffer) + 1);
    memset(response_buffer, 0, BUFFER_SIZE);
}

void handle_connection(const int incoming_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(incoming_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("Received %zd bytes: %s\n", bytes_received, buffer);

        buffer[strcspn(buffer, "\r\n")] = 0;
        to_uppercase(buffer);

        if (bytes_received > 1) {
            if (strcmp(buffer,"QUIT") == 0) {
                break;
            }

            handle_command(incoming_fd, buffer);
        }
        // Clear buffer after processing to receive next chunk of data
        memset(buffer, 0, BUFFER_SIZE);

        if (bytes_received == -1) {
            perror("recv failed");
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
        }
    }
}

int main() {
    int server_fd, incoming_fd;
    const int socket_opt = 1;
    struct sockaddr_in socket_address;
    int addr_size = sizeof(socket_address);

    // Create the socked fd
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options for address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_opt, sizeof(socket_opt))) {
        perror("Could not set the socket options");
        exit(EXIT_FAILURE);
    }

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(CLIENT_PORT);

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *) &socket_address, addr_size)) {
        perror("Could not bind to network address");
        exit(EXIT_FAILURE);
    }

    // Listen to connections
    if (listen(server_fd, 2)) {
        perror("Could not listen to connections");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept any incoming connection
        if ((incoming_fd = accept(server_fd, (struct sockaddr*) &socket_address, (socklen_t*) &addr_size)) < 0) {
            perror("Could not accept incoming connections");
            exit(EXIT_FAILURE);
        }

        // handle the messages with the client
        handle_connection(incoming_fd);

        // close incoming connection
        close(incoming_fd);
        break;
    }

    // close all server connection
    close(server_fd);

    return 0;
}