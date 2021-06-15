#include <server/server.h>

#include <fcntl.h>
#include <stdio.h>

#include <server/server_logic.h>

void start_server(in_port_t port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;

    int one = 1;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == 0) {
        perror("Failed to create socket");
        return;
    }

    int setsockopt_rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (setsockopt_rc != 0) {
        perror("Failed to configure socket");
        return;
    }

    int bind_rc = bind(socket_fd, (struct sockaddr*) &address, sizeof address);
    if (bind_rc < 0) {
        perror("Filed to bind a socket");
        return;
    }

    int listen_rc = listen(socket_fd, 1024);
    if (listen_rc < 0) {
        perror("Failed to enter passive socket mode");
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

    printf("Server started, port = %d\n", ntohs(port));
    server_start_service(socket_fd, port);
}