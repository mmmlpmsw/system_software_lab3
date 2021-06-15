#pragma once

#include <netinet/in.h>

void start_client(const char* user, struct in_addr ipv4, in_port_t port);