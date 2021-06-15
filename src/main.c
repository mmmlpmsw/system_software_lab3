#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <client/client.h>
#include <server/server.h>

#define CLIENT_MODE_KEY "--client"
#define SERVER_MODE_KEY "--server"
#define HELP_MODE_KEY   "--help"

void help(const char* executable);
int main_client(int argc, const char* argv[]);
int main_server(int argc, const char* argv[]);

int main(int argc, const char* argv[]) {
    const char* executable = argv[0];
    if (argc < 2) {
        help(executable);
    } else {
        const char* mode = argv[1];
        if (strcmp(mode, CLIENT_MODE_KEY) == 0) {
            return main_client(argc, argv);
        } else if (strcmp(mode, SERVER_MODE_KEY) == 0) {
            return main_server(argc, argv);
        } else if (strcmp(mode, HELP_MODE_KEY) == 0)
            help(executable);
        else
            puts("Unknown mode, use --help for help.");
    }
    return 0;
}

int main_client(int argc, const char* argv[]) {
    if (argc < 5) {
        puts("Not enough arguments. Use --help option for help.");
        return -1;
    }
    const char* user_name = argv[2];
    const char* ip_raw = argv[3];
    const char* port_raw = argv[4];
    struct in_addr ip;
    in_port_t port;
    if (inet_pton(AF_INET, ip_raw, &ip) <= 0) {
        puts("Invalid ip address.");
        return -1;
    }
    char* strtol_endptr;
    long int port_unchecked = strtol(port_raw, &strtol_endptr, 10);
    if (*strtol_endptr != '\0') {
        puts("Port should be an integer");
        return 0;
    }
    if (port_unchecked < 1 || port_unchecked > 65535) {
        puts("Port should be between 1 and 65535");
        return 0;
    }
    port = htons(port_unchecked);
    start_client(user_name, ip, port);
    return 0;
}

int main_server(int argc, const char* argv[]) {
    if (argc < 3) {
        puts("Not enough arguments. Use --help option for help.");
        return -1;
    }
    const char* port_raw = argv[2];
    in_port_t port;
    char* strtol_endptr;
    long int port_unchecked = strtol(port_raw, &strtol_endptr, 10);
    if (*strtol_endptr != '\0') {
        puts("Port should be an integer");
        return 0;
    }
    if (port_unchecked < 1 || port_unchecked > 65535) {
        puts("Port should be between 1 and 65535");
        return 0;
    }
    port = htons(port_unchecked);
    start_server(port);
    return 0;
}

void help(const char* executable) {
    printf(
            "Usage:\n"
            "%s --client <user> <ip_address> <port>\n"
            "%s --server <port>\n"
            "%s --help\n",
            executable,
            executable,
            executable
    );
}