#include <client/input/client_delete_listener.h>
#include <client/client_state.h>
#include <api.h>
#include <string.h>

void client_delete_listener(int key, client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_LISTS && state->lists != NULL) {
        message_delete_list msg = (message_delete_list) {
                .header = {
                        .type = DELETE_LIST,
                        .size = sizeof (message_delete_list)
                },
                .name = { 0 }
        };
        strcpy(msg.name, get_list_by_index(state, state->selected_list)->name);
        api_send(state->socket_fd, (message_header*) &msg);
        state->selected_list--;
        if (state->selected_list < 0)
            state->selected_list = 0;
    } else if (state->focused_window == CLIENT_WINDOW_TASKS && get_list_by_index(state, state->selected_list)->tasks != NULL) {
        message_delete_task msg = (message_delete_task) {
                .header = {
                        .type = DELETE_TASK,
                        .size = sizeof (message_delete_task)
                },
                .list_name = { 0 },
                .task_name = { 0 }
        };
        strcpy(msg.list_name, get_list_by_index(state, state->selected_list)->name);
        strcpy(msg.task_name, get_task_by_index(get_list_by_index(state, state->selected_list), state->selected_task)->name);
        api_send(state->socket_fd, (message_header*) &msg);
        state->selected_task--;
    }
};