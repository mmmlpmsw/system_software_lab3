#include <client/input/client_edit_listener.h>
#include <client/input/client_input.h>
#include <constraints.h>
#include <string.h>
#include <time.h>
#include <api.h>

void on_edit_complete(client_state* state) {
    state->focused_window = CLIENT_WINDOW_TASKS_DESCRIPTION;
    if (strlen(state->buffer) == 0)
        return;
    list* selected_list = get_list_by_index(state, state->selected_list);
    task* selected_task = get_task_by_index(selected_list, state->selected_task);
    char old_name[MAX_NAME_SIZE + 1] = {0};
    strcpy(old_name, selected_task->name);
    if (state->selected_field == 0)
        strcpy(selected_task->name, state->buffer);
    else if (state->selected_field == 1)
        strcpy(selected_task->description, state->buffer);
    else if (state->selected_field == 2) {
        struct tm tm;
        strptime(state->buffer, "%H:%M:%S %d.%m.%Y", &tm);
        selected_task->deadline = mktime(&tm);
    } else return;
    message_update_task msg = {
            .header = (message_header) {
                    .type = UPDATE_TASK,
                    .size = sizeof(message_update_task)
            },
            .name = { 0 },
            .new_name = { 0 },
            .list_name = { 0 },
            .description = { 0 }
    };

    strcpy(msg.name, old_name);
    strcpy(msg.new_name, selected_task->name);
    strcpy(msg.list_name, selected_list->name);
    strcpy(msg.description, selected_task->description);
    msg.deadline = selected_task->deadline;

    api_send(state->socket_fd, (message_header*) &msg);
}

void client_edit_listener(int key, client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION) {
        list* selected_list = get_list_by_index(state, state->selected_list);
        task* selected_task = get_task_by_index(selected_list, state->selected_task);
        if (state->selected_field == 0) {
            get_input(state, MAX_NAME_SIZE, on_edit_complete, "Enter name: ");
            strcat(state->buffer, selected_task->name);
        } else if (state->selected_field == 1) {
            get_input(state, MAX_DESCRIPTION_SIZE, on_edit_complete, "Enter description: ");
            strcat(state->buffer, selected_task->description);
        } else if (state->selected_field == 2) {
            get_input(state, MAX_NAME_SIZE, on_edit_complete, "Enter deadline: ");
            char buffer[MAX_NAME_SIZE] = {0};
            strftime(buffer, MAX_NAME_SIZE, "%H:%M:%S %d.%m.%Y", localtime(&selected_task->deadline));
            strcat(state->buffer, buffer);
        }
        state->caret_position = (int) strlen(state->buffer);
    }
}