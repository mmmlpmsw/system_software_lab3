#include <server/server_logic.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <api.h>
#include <stdio.h>

server_state* state;

void server_stop_interrupt_listener(int signum_ignored);
void server_dispose_state();
void process_requests();

void server_start_service(int socket_fd, in_port_t port) {
    signal(SIGINT, server_stop_interrupt_listener);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;
    socklen_t address_len = sizeof(address);

    state = malloc(sizeof(server_state));
    state->keep_running = true;
    state->users = NULL;

    while (state->keep_running) {
        int new_client_socket_fd = accept(socket_fd, (struct sockaddr*) &address, &address_len);
        if (new_client_socket_fd >= 0) {
            printf("New anonymous user connected\n");
            add_user(state, new_client_socket_fd);
        }

        process_requests();
    }

    server_dispose_state();
}

void process_user_message(server_connected_user* user, message_header* message) {
    if (message->type == HELLO) {
        message_hello* hello = (message_hello*) message;
        user->name = malloc(MAX_NAME_SIZE + 1);
        strncpy(user->name, hello->user_name, MAX_NAME_SIZE);
        server_connected_user* existing_user = find_user_by_name(state, user->name);
        if (existing_user)
            user->lists = existing_user->lists;
        printf("User logged in: %s\n", user->name);
        api_do_hello_response(user->socket_fd, OK);
    } else if (message->type == BYE) {
        printf(user->name == NULL ? "Anonymous user disconnected\n" : "User '%s' disconnected\n", user->name);
        remove_user(state, user);
    } else if (message->type == GET_LISTS) {
        printf("User '%s' requested lists\n", user->name);
        api_send_lists_list(user->socket_fd, state);
    } else if (message->type == CREATE_LIST) {
        if (user->name == NULL)
            return;
        message_create_list* create_message = (message_create_list*) message;
        create_list(user, create_message->name);
        printf("User '%s' created a new task list with name '%s'\n", user->name, create_message->name);
        api_send_lists_list(user->socket_fd, state);
    } else if (message->type == CREATE_TASK) {
        if (user->name == NULL)
            return;
        message_create_task* create_message = (message_create_task*) message;
        create_task(user, create_message->name, create_message->description, create_message->list_name,
                    create_message->deadline);
        printf("User '%s' created a new task with name '%s'\n", user->name, create_message->name);
        api_send_lists_list(user->socket_fd, state);
    } else if (message->type == UPDATE_TASK) {
        if (user->name == NULL)
            return;
        message_update_task * update_message = (message_update_task*) message;
        update_task(user, update_message->name, update_message->new_name, update_message->description, update_message->list_name,
                    update_message->deadline);
        printf("User '%s' updated a task with name '%s'\n", user->name, update_message->name);
        api_send_lists_list(user->socket_fd, state);
    } else if (message->type == DELETE_LIST) {
        if (user->name == NULL)
            return;
        message_delete_list * delete_message = (message_delete_list*) message;
        delete_list(user, delete_message->name);
        printf("User '%s' delete a list with name '%s'\n", user->name, delete_message->name);
        api_send_lists_list(user->socket_fd, state);
    } else if (message->type == DELETE_TASK) {
        if (user->name == NULL)
            return;
        message_delete_task * delete_message = (message_delete_task*) message;
        delete_task(user, delete_message->task_name, delete_message->list_name);
        printf("User '%s' delete a task with name '%s'\n", user->name, delete_message->task_name);
        api_send_lists_list(user->socket_fd, state);
    }
    char* name = user->name;
    server_connected_user* current = state->users;
    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            api_send_lists_list(current->socket_fd, state);
        }
        current = current->next;
    }
}

void process_requests() {
    server_connected_user* user = state->users;
    while (user != NULL) {
//        if (!is_socket_alive(user->socket_fd)) {
//            printf(user->name == NULL ? "Anonymous user connection lost\n" : "User '%s' connection lost\n", user->name);
//            server_connected_user* next = user->next;
//            remove_user(state, user);
//            user = next;
//            continue;
//        }
        message_header* message = api_get_next_message(user->socket_fd);
        server_connected_user* next = user->next;
        if (message != NULL)
            process_user_message(user, message); // May remove current user
        free(message);
        user = next;
    }
}

void server_stop_service() {
    puts("\nStopping server...");
    state->keep_running = false;
}

void server_stop_interrupt_listener(__attribute__((unused)) int signum_ignored) {
    server_stop_service();
}

void server_dispose_state() {
    server_connected_user* user = state->users;
    while (user != NULL) {
        server_connected_user* next = user->next;
        close(user->socket_fd);
        remove_user(state, user);
        user = next;
    }
    free(state);
}