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

void handle_connection(char buffer[], const int client_fd) {
    send_message(client_fd, buffer, strlen(buffer) + 1);

    int bytes_received;
    if ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        fprintf(stdin,"Received %zd bytes: %s\n", bytes_received, buffer);

        buffer[strcspn(buffer, "\r\n")] = 0;
        to_uppercase(buffer);

        // Clear buffer after processing to receive next chunk of data
        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Host was not provided\n");
        fprintf(stderr, "Usage ./ftp_client <host>\n");
        exit(EXIT_FAILURE);
    }

    // TODO validate that the host is actually correctly set
    const char* host = argv[1];

    int client_fd;
    struct sockaddr_in socket_address;
    const int addr_size = sizeof(socket_address);

    // Create the socked fd
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = inet_addr(host);
    socket_address.sin_port = htons(CLIENT_PORT);

    // Connect the socket to the address and port
    fprintf(stdout, "Connecting to host: %s\n", host);
    if (connect(client_fd, (struct sockaddr *) &socket_address, addr_size)) {
        perror("Could not bind to network address");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        char buffer[BUFFER_SIZE];

        // Read data in from the console
        if(fgets(buffer,BUFFER_SIZE ,stdin) != NULL) {
            // Remove trailling new line from the buffer
            buffer[strcspn(buffer, "\r\n")] = 0;

            to_uppercase(buffer);
            if (strcmp(buffer, "QUIT") == 0) break;
            // todo should i send quit to the server as well?

            handle_connection(buffer, client_fd);
        }
    }

    // close all server connection
    close(client_fd);

    return 0;
}