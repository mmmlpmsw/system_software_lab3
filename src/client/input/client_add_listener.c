#include <client/client_state.h>
#include <api.h>
#include <client/input/client_input.h>
#include <string.h>
#include <stdio.h>

#define _XOPEN_SOURCE
#include <time.h>

void on_input_lists(client_state* state) {
    state->focused_window = CLIENT_WINDOW_LISTS;
    if (strlen(state->buffer) == 0)
        return;
    message_create_list msg = {
            .header = (message_header) {
                    .type = CREATE_LIST,
                    .size = sizeof(message_create_list)
            },
            .name = { 0 }
    };
    strcpy(msg.name, state->buffer);
    api_send(state->socket_fd, (message_header*) &msg);
}

void on_input_task_deadline(client_state* state) {
    fprintf(stderr, "We got into on_input_task_deadline\n");
    state->focused_window = CLIENT_WINDOW_TASKS;
    message_create_task msg = {
            .header = (message_header) {
                    .type = CREATE_TASK,
                    .size = sizeof(message_create_task)
            },
            .name = { 0 },
            .list_name = { 0 },
            .description = { 0 }
    };

    task* new_task = get_last_task(get_list_by_index(state, state->selected_list));
    strcpy(msg.name, new_task->name);
    strcpy(msg.list_name, get_list_by_index(state, state->selected_list)->name);
    if (new_task->description != NULL)
        strcpy(msg.description, new_task->description);
    struct tm tm;
    strptime(state->buffer, "%H:%M:%S %d.%m.%Y", &tm);
    msg.deadline = mktime(&tm);

    api_send(state->socket_fd, (message_header*) &msg);
    fprintf(stderr, "Message sent\n");
    while (state->lists != NULL)
        client_delete_lists(state);
}

void on_input_task_description(client_state* state) { //todo (seems like done)
    fprintf(stderr, "We got into input_task_description\n");
    list* current_list = get_list_by_index(state, state->selected_list);
    task* new_task = get_last_task(current_list);
    new_task->description = strdup(state->buffer);

    get_input(state, MAX_DESCRIPTION_SIZE, on_input_task_deadline, "Enter deadline: ");
}

void on_input_task_name(client_state* state) { //todo (seems like done)
    fprintf(stderr, "We got into input_task_name\n");
    if (strlen(state->buffer) == 0)
        return;
    list* current_list = get_list_by_index(state, state->selected_list);
    task* last_task = get_last_task(current_list);
    task* new_task = malloc(sizeof (task));
    new_task->name = strdup(state->buffer);
    new_task->list_name = strdup(current_list->name);
    new_task->next = NULL;
    new_task->description = NULL;
    if (last_task != NULL)
        last_task->next = new_task;
    else
        current_list->tasks = new_task;

    fprintf(stderr, "Going into on_input_description\n");
    get_input(state, MAX_DESCRIPTION_SIZE, on_input_task_description, "Enter description:");
    fprintf(stderr, "Went out from on_input_task_name\n");

    /*if (strlen(state->buffer) == 0)
        return;
    message_create_task msg = {
            .header = (message_header) {
                    .type = CREATE_TASK,
                    .size = sizeof(message_create_task)
            },
            .name = { 0 },
            .list_name = { 0 },
            .description = { 0 },
            .deadline = time(NULL)
    };
    strcpy(msg.name, new_task->name);
    strcpy(msg.list_name, (get_list_by_index(state, state->selected_list))->name);
    strcpy(msg.description, "default");

    api_send(state->socket_fd, (message_header*) &msg);*/
}

void client_add_listener(int key, client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_LISTS)
        get_input(state, MAX_NAME_SIZE, on_input_lists, "Enter name:");
    else if (state->focused_window == CLIENT_WINDOW_TASKS)
        get_input(state, MAX_NAME_SIZE, on_input_task_name, "Enter name:");
}