#pragma once

#include <netinet/in.h>

void server_start_service(int socket_fd, in_port_t port);