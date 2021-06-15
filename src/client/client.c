#include <client/client.h>

#include <stdio.h>
#include <fcntl.h>

#include <client/client_logic.h>

void start_client(const char* user, struct in_addr ipv4, in_port_t port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr = ipv4;
    address.sin_port = port;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == 0) {
        perror("Failed to create socket");
        return;
    }

    puts("Connecting...");

    int connect_rc = connect(socket_fd, (struct sockaddr*) &address, sizeof(address));
    if (connect_rc < 0) {
        perror("Could not connect to server");
        return;
    }

    int flags = fcntl(socket_fd, F_GETFL);
    if (flags == -1) {
        perror("Could not get socket flags");
        return;
    }

    int fcntl_rc = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    if (fcntl_rc == -1) {
        perror("Could not set socket flags");
        return;
    }

    client_start_service(socket_fd, user);
}