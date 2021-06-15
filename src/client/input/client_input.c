#include <client/input/client_input.h>

#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

void function_that_does_nothing(int key, client_state* state) {}

typedef struct key_listener_node key_listener_node;
struct key_listener_node {
    key_listener_function function;
    key_listener_node* next;
};

key_listener_function any_key_listener = function_that_does_nothing;

key_listener_node* key_listeners_by_key_code[256] = { NULL };

void add_any_key_listener(key_listener_function function) {
    any_key_listener = function != NULL ? function : function_that_does_nothing;
}

void add_key_listener(int key, key_listener_function function) {
    key_listener_node* new_node = malloc(sizeof(key_listener_node));
    new_node->next = NULL;
    new_node->function = function;
    key_listener_node** target = &key_listeners_by_key_code[key];
    while (*target != NULL)
        target = &(*target)->next;
    *target = new_node;
}

void process_key(client_state* state, int key) {
    key_listener_node* target = key_listeners_by_key_code[key];
    while (target != NULL) {
        target->function(key, state);
        target = target->next;
    }
    any_key_listener(key, state);
}

bool process_user_input(client_state* state) {
    fflush(stdin);
    int bytes_available = 0;
    ioctl(0, FIONREAD, &bytes_available);
    if (bytes_available == 0)
        return false;
    process_key(state, getchar());
    return true;
}

void get_input(client_state* state, size_t max_length, void (*input_listener)(client_state*), char* prompt) {
    state->prompt = strdup(prompt);
    state->focused_window = CLIENT_WINDOW_EDIT;
    if (state->buffer != NULL)
        free(state->buffer);
    state->buffer = malloc(max_length + 1);
    state->buffer_limit = max_length;
    memset(state->buffer, 0, max_length + 1);
    state->input_listener = input_listener;
    state->caret_position = 0;
}