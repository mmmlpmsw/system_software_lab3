#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include <common_state.h>

typedef struct server_connected_user server_connected_user;
struct server_connected_user {
    int socket_fd;
    char* name;
    list* lists;
    server_connected_user* next;
};

typedef struct {
    bool keep_running;
    server_connected_user* users;
    server_connected_user* users_archived;
} server_state;

server_connected_user* find_user_by_fd(server_state* state, int socket_fd);
server_connected_user* find_user_by_name(server_state* state, char* name);
void add_user(server_state* state, int socket_fd);
void remove_user(server_state* state, server_connected_user* user);
void create_task(server_connected_user *user, char *name, char *description, char *list_name, time_t deadline);
void update_task(server_connected_user *user, char* old_name, char *new_name, char *description, char *list_name, time_t deadline);
void delete_task(server_connected_user *user, char *name, char *list_name);
void create_list(server_connected_user *user, char *name);
void delete_list(server_connected_user *user, char *name);