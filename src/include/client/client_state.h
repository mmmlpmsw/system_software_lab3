#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <common_state.h>

typedef enum {
    CLIENT_WINDOW_LISTS,
    CLIENT_WINDOW_TASKS,
    CLIENT_WINDOW_TASKS_DESCRIPTION,
    CLIENT_WINDOW_EDIT
} client_window_type;

typedef enum {
    CONNECTED,
    DISCONNECTED
} client_connection_status;

typedef struct client_state client_state;
struct client_state {
    bool keep_running;
    client_connection_status connection_status;
    int socket_fd;
    const char* user;
    char* prompt;

    client_window_type focused_window;
    list* lists;
    int selected_task;
    int selected_list;
    int selected_field;
    char * buffer;
    size_t buffer_limit;
    int caret_position;
    void (*input_listener)(client_state*);
};

void client_delete_lists(client_state *state);
void client_create_list(client_state *state, char *name);
void client_create_task(client_state* state, char *name, time_t initial_time, char *description, char *list_name, time_t deadline);

int count_lists(client_state* state);
int count_tasks(list * l);
list* get_list_by_index(client_state* state, int list_index);
task* get_task_by_index(list* l, int task_index);