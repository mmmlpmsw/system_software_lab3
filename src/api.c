#include <api.h>

#include <sys/time.h>
#include <memory.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <common_state.h>

#define DEFAULT_TIMEOUT 10000

void null_fail_listener() {}

typedef struct message_listener_node message_listener_node;
struct message_listener_node {
    message_listener_function function;
    message_listener_node* next;
};

message_listener_node* message_listener_by_type[LAST_] = { NULL };
connection_fail_listener_function on_connection_fail = null_fail_listener;

void api_set_connection_fail_listener(connection_fail_listener_function listener) {
    on_connection_fail = listener;
}

void api_add_message_listener(message_type type, message_listener_function listener) {
    message_listener_node* new_node = malloc(sizeof(message_listener_node));
    new_node->next = NULL;
    new_node->function = listener;

    if (message_listener_by_type[type] == NULL) {
        message_listener_by_type[type] = new_node;
    } else {
        message_listener_node* current = message_listener_by_type[type];
        while (current->next != NULL)
            current = current->next;
        current->next = new_node;
    }
}

message_header* api_get_next_message(int socket_fd) {
    if (socket_fd == -1)
        return NULL;

    int bytes_available = 0;
    ioctl(socket_fd, FIONREAD, &bytes_available);
    if (bytes_available == 0)
        return NULL;

    message_header header;
    ssize_t have_read = read(socket_fd, &header, sizeof(header));

    if (have_read == -1 && errno == EWOULDBLOCK) // No data
        return NULL;

    while (have_read != sizeof(header)) {
        ssize_t new_read = read(socket_fd, ((uint8_t*)&header) + have_read, sizeof(header) - have_read);
        if (new_read == -1 && errno != EWOULDBLOCK) {
            on_connection_fail();
            return NULL;
        }
        if (new_read > 0)
            have_read += new_read;
    }

    message_header* message = malloc(header.size);
    memcpy(message, &header, sizeof(header));
    size_t body_size = header.size - sizeof(header);
    have_read = read(socket_fd, ((uint8_t*) message) + sizeof(header), body_size);

    while (have_read != body_size) {
        ssize_t new_read = read(socket_fd, ((uint8_t*) message) + sizeof(header) + have_read, body_size - have_read);
        if (new_read == -1 && errno != EWOULDBLOCK) {
            free(message);
            on_connection_fail();
            return NULL;
        }
        if (new_read > 0)
            have_read += new_read;
    }

    return message;
}

void process_new_message(message_header* message) {
    message_listener_node* current_listener = message_listener_by_type[message->type];
    while (current_listener != NULL) {
        current_listener->function(message);
        current_listener = current_listener->next;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointers" // CLion bug
void process_api_messages(int socket_fd) {
    message_header* new_message;
    while ((new_message = api_get_next_message(socket_fd)) != NULL) {
        process_new_message(new_message);
        free(new_message);
    }
}
#pragma clang diagnostic pop

void api_send(int socket_fd, message_header* message) {
    size_t wrote = 0;
    while (wrote < message->size) {
        size_t new_wrote = write(socket_fd, message, message->size);
        wrote += new_wrote;
        if (new_wrote == -1) {
            on_connection_fail();
            break;
        }
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointers" // CLion bug
message_header* api_send_and_wait(
        int socket_fd,
        message_header* message,
        message_type response_type,
        uint64_t timeout_millis
) {
    struct timeval start, current;
    gettimeofday(&start, NULL);
    api_send(socket_fd, message);
    message_header *new_message = NULL;
    do {
        if (new_message != NULL)
            free(new_message);

        if ((new_message = api_get_next_message(socket_fd)) != NULL)
            process_new_message(new_message);

        gettimeofday(&current, NULL);
        long working_time = (current.tv_sec - start.tv_sec) * 1000000 + current.tv_usec - start.tv_usec;
        if (working_time > timeout_millis) { // Timeout
            free(new_message);
            on_connection_fail();
            return NULL;
        }
    } while (new_message == NULL || new_message->type != response_type);
    return new_message;
}

message_hello_response* api_do_hello(int socket_fd, const char* user) {
    message_hello* msg = malloc(sizeof(message_hello));
    msg->header.type = HELLO;
    msg->header.size = sizeof(message_hello);
    strncpy(msg->user_name, user, MAX_NAME_SIZE + 1);
    message_hello_response* response = (message_hello_response*) api_send_and_wait(
            socket_fd,
            (message_header*) msg,
            HELLO_RESPONSE,
            DEFAULT_TIMEOUT
    );
    free(msg);
    return response;
}
#pragma clang diagnostic pop

void api_do_hello_response(int socket_fd, hello_response_type type) {
    size_t msg_size = sizeof(message_hello_response);
    message_hello_response* msg = malloc(msg_size);
    msg->header.type = HELLO_RESPONSE;
    msg->header.size = msg_size;
    msg->response_type = type;
    api_send(socket_fd, (message_header*) msg);
    free(msg);
}

server_connected_user* get_user_by_socket_fd(server_state* state, int socket_fd) {
    server_connected_user* current = state->users;
    while (current != NULL && current->socket_fd != socket_fd)
        current = current->next;
    return current;
}

int count_user_lists(server_connected_user* user) {
    int count = 0;
    list* current = user->lists;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

int count_total_user_tasks(server_connected_user* user) {
    int count = 0;
    list* current_list = user->lists;
    while (current_list != NULL) {
        task* current_task = current_list->tasks;
        while (current_task != NULL) {
            count++;
            current_task = current_task->next;
        }
        current_list = current_list->next;
    }
    return count;
}

void api_send_lists_list(int socket_fd, server_state* state) {
    server_connected_user* user = get_user_by_socket_fd(state, socket_fd);
    if (user == NULL)
        return;
    size_t lists_size = MAX_NAME_SIZE * count_user_lists(user);
    size_t tasks_total_size = sizeof(task_dto) * count_total_user_tasks(user);
    size_t message_size = sizeof(message_lists) + lists_size + tasks_total_size;
    message_lists* message = malloc(message_size);
    message->lists_size = count_user_lists(user);

    char* lists_entry = (char*)(((uint8_t*) message) + sizeof(message_lists));
    int list_index = 0;
    list* current_list = user->lists;
    while (current_list != NULL) {
        strncpy(lists_entry + (MAX_NAME_SIZE) * list_index, current_list->name, MAX_NAME_SIZE);
        list_index++;
        current_list = current_list->next;
    }

    task_dto* tasks_entry = (task_dto*) (((uint8_t*) message) + sizeof(message_lists) + lists_size);
    current_list = user->lists;
    int task_index = 0;
    while (current_list != NULL) {
        task* current_task = current_list->tasks;
        while (current_task != NULL) {
            tasks_entry[task_index].deadline = current_task->deadline;
            tasks_entry[task_index].initial_time = current_task->initial_time;
            strncpy(tasks_entry[task_index].name, current_task->name, MAX_NAME_SIZE);
            strncpy(tasks_entry[task_index].list_name, current_task->list_name, MAX_NAME_SIZE);
            strncpy(tasks_entry[task_index].description, current_task->description, MAX_DESCRIPTION_SIZE);
            task_index++;
            current_task = current_task->next;
        }
        current_list = current_list->next;
    }

    message->header.size = message_size;
    message->header.type = LISTS_LIST;
    api_send(socket_fd, (message_header*) message);
    free(message);
}

bool is_socket_alive(int socket_fd) {
    if (socket_fd == -1)
        return false;
    int error = 0;
    socklen_t len = sizeof error;
    return getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0;
}