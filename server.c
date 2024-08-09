#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "stdio.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#include <unistd.h>


#define FTP_PORT 21
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

int main() {
    int server_fd, listening_socket;
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
    socket_address.sin_port = htons(FTP_PORT);

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
        if ((listening_socket = accept(server_fd, (struct sockaddr*) &socket_address, (socklen_t*) &addr_size)) < 0) {
            perror("Could not accept incoming connections");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];

        ssize_t bytes_received;
        while ((bytes_received = recv(listening_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            printf("Received %zd bytes: %s\n", bytes_received, buffer);
            to_uppercase(buffer);
            if (strcmp(buffer,"QUIT\n") == 0) {
                break;
            }
            ssize_t total_bytes_sent = 0;
            while (total_bytes_sent < bytes_received) {
                ssize_t bytes_sent = send(listening_socket, buffer + total_bytes_sent, bytes_received - total_bytes_sent, 0);
                if (bytes_sent == -1) {
                    perror("The send function failed");
                    close(listening_socket);
                    exit(EXIT_FAILURE);
                }
                total_bytes_sent += bytes_sent;
            }

            // Clear buffer after processing to receive next chunk of data
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (bytes_received == -1) {
            perror("recv failed");
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
        }
        close(listening_socket);
        break;
    }


    // send(listening_socket, )

    // close all the open connections
    close(server_fd);

    return 0;
}