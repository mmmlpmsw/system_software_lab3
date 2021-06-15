#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <constraints.h>

#include <server/server_state.h>

typedef enum {
    HELLO,  // Login
    GET_LISTS,
    CREATE_TASK,
    DELETE_TASK,
    UPDATE_TASK,
    CREATE_LIST,
    DELETE_LIST,
    BYE,

    HELLO_RESPONSE, // Login response
    LISTS_LIST,

    LAST_ // Used to determine enum size
} message_type;

typedef enum {
    OK, INVALID_NAME, USER_EXISTS
} hello_response_type;

typedef struct {
    uint32_t size; // Size of the whole message
    message_type type;
} message_header;

typedef struct {
    message_header header;
    char user_name[MAX_NAME_SIZE + 1];
} message_hello;

typedef struct {
    message_header header;
    char name[MAX_NAME_SIZE + 1];
} message_create_list;
typedef message_create_list message_delete_list;

typedef struct {
    message_header header;
    char list_name[MAX_NAME_SIZE + 1];
    char task_name[MAX_NAME_SIZE + 1];
} message_delete_task;

typedef struct {
    message_header header;
    char name[MAX_NAME_SIZE + 1];
    char new_name[MAX_NAME_SIZE + 1];
    char description[MAX_DESCRIPTION_SIZE];
    char list_name[MAX_NAME_SIZE + 1];
    time_t deadline;
} message_create_task;
typedef message_create_task message_update_task;

typedef struct {
    message_header header;
    hello_response_type response_type;
} message_hello_response;

typedef struct {
    char name[MAX_NAME_SIZE];
    char description[MAX_DESCRIPTION_SIZE];
    char list_name[MAX_NAME_SIZE];
    time_t deadline;
    time_t initial_time;
} task_dto;

typedef struct {
    message_header header;
    int lists_size;
    // names list of size MAX_NAME_SIZE + 1
    // tasks list (task_dto)
} message_lists;

typedef void (*message_listener_function)(const message_header*);
typedef void (*connection_fail_listener_function)();

bool is_socket_alive(int socket_fd);

void api_set_connection_fail_listener(connection_fail_listener_function listener);
void api_add_message_listener(message_type type, message_listener_function listener);
void process_api_messages(int socket_fd);
void api_send(int socket_fd, message_header* message);
message_header* api_get_next_message(int socket_fd);
message_header* api_send_and_wait(int socket_fd, message_header* message, message_type response_type, uint64_t timeout_millis);

message_hello_response* api_do_hello(int socket_fd, const char* user);
void api_do_hello_response(int socket_fd, hello_response_type type);
void api_send_lists_list(int socket_fd, server_state* state);