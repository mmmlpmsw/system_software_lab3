#include <client/client_logic.h>

#include <stdbool.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <string.h>

#include <api.h>
#include <util/mem_util.h>
#include <client/ui/renderer.h>
#include <client/input/client_input.h>
#include <client/input/client_tab_listener.h>
#include <client/input/client_arrows_listener.h>
#include <client/input/client_add_listener.h>
#include <stdio.h>
#include <client/input/client_text_editor_input_input.h>
#include <client/input/client_enter_listener.h>
#include <client/input/client_delete_listener.h>
#include <client/input/client_edit_listener.h>

struct termios terminal_settings_original;
client_state* state = NULL;

void client_stop_interrupt_listener(int signum);
void client_connection_fail_listener();
void client_start_gui();
void client_stop_gui();
void client_init_api_listeners();
void client_init_key_listeners();
void client_init_state(int socket_fd, const char* user);
void client_dispose_state();

void client_start_service(int socket_fd, const char* user) {
    signal(SIGINT, client_stop_interrupt_listener);

    api_set_connection_fail_listener(client_connection_fail_listener);
    puts("Logging in...");
    message_hello_response* hello_res = api_do_hello(socket_fd, user);
    if (hello_res == NULL)
        puts("Could not log in: server did not respond properly");
    else if (hello_res->response_type == INVALID_NAME)
        puts("Could not log in: invalid user name");
    else if (hello_res->response_type == USER_EXISTS)
        puts("Could not log in: user with this name is already logged in");
    else {
        puts("Logged in!");
        client_init_state(socket_fd, user);
        client_init_api_listeners();
        client_init_key_listeners();
        client_start_gui();
        client_stop_gui();

        if (state->connection_status == DISCONNECTED)
            puts("Disconnected");

        client_dispose_state();
    }
    free(hello_res);
}

void client_init_state(int socket_fd, const char* user) {
    state = malloc(sizeof(client_state));
    state->keep_running = true;
    state->connection_status = CONNECTED;
    state->socket_fd = socket_fd;
    state->lists = NULL;
    state->focused_window = CLIENT_WINDOW_LISTS;
    state->user = user;
    state->selected_field = 0;
    state->selected_list = 0;
    state->selected_task = 0;
    state->buffer = NULL;
    state->caret_position = 0;
    state->prompt = NULL;
}

void client_stop_service() {
    if (state != NULL) {
        if (state->connection_status == CONNECTED)
            api_send(state->socket_fd, &(message_header) {.size = sizeof(message_header), .type = BYE});
        if (state->keep_running)
            client_stop_gui();
    }
}

void on_lists_message(const message_header* message_header) {
    client_delete_lists(state);
    message_lists* message = (message_lists*) message_header;
    //fprintf(stderr,"listener: bytes %s\n",message);
    uint32_t lists_length = message->lists_size;

    char* lists_entry = (char*)(((uint8_t*) message) + sizeof(message_lists));
    for (int i = 0; i < lists_length; i++) {
        char* list_name = malloc(MAX_NAME_SIZE + 1);
        strncpy(list_name, lists_entry + (MAX_NAME_SIZE) * i, MAX_NAME_SIZE + 1);
        client_create_list(state, list_name);
    }

    task_dto* tasks_entry = (task_dto*) (((uint8_t*) message) + sizeof(message_lists) + MAX_NAME_SIZE*lists_length);
    uint32_t tasks_count = (message->header.size - sizeof(message_lists) - MAX_NAME_SIZE * lists_length)/ sizeof(task_dto);

    for (int i = 0; i < tasks_count; i++) {
        task_dto* current_task = &tasks_entry[i];
        char* name = malloc(MAX_NAME_SIZE + 1);
        char* list_name = malloc(MAX_NAME_SIZE + 1);
        char* description = malloc(MAX_DESCRIPTION_SIZE + 1);

        fprintf(stderr,"receive list %s\n",current_task->list_name);
        fprintf(stderr,"receive task %s\n",current_task->name);
        fprintf(stderr,"receive task description %s\n",current_task->description);
        fprintf(stderr,"receive task init time %ld\n",current_task->initial_time);
        fprintf(stderr,"receive task deadline time %ld\n",current_task->deadline);

        strncpy(name, current_task->name, MAX_NAME_SIZE);
        strncpy(list_name, current_task->list_name, MAX_NAME_SIZE);
        strncpy(description, current_task->description, MAX_DESCRIPTION_SIZE);
        client_create_task(state, name, current_task->initial_time, description, list_name, current_task->deadline);
    }
}

void client_init_api_listeners() {
    api_add_message_listener(LISTS_LIST, on_lists_message);
}

void client_init_key_listeners() {
    add_key_listener(INPUT_TAB, client_tab_listener);
    add_key_listener(INPUT_ESCAPE, client_tab_listener);
    add_key_listener(INPUT_CTRL_Q, client_stop_service);
    add_key_listener(INPUT_ESCAPE, client_arrows_listener);
    add_key_listener(INPUT_CTRL_N, client_add_listener);
    add_key_listener(INPUT_ENTER, client_enter_listener);
    add_key_listener(INPUT_CTRL_D, client_delete_listener);
    add_key_listener(INPUT_CTRL_E, client_edit_listener);
    add_any_key_listener(client_text_editor_input_listener);
}

void client_start_gui() {
    struct termios terminal_settings;
    tcgetattr(0, &terminal_settings);
    tcgetattr(0, &terminal_settings_original);
    cfmakeraw(&terminal_settings);
    tcsetattr(0, TCSANOW, &terminal_settings);
    hide_cursor();

    api_send(state->socket_fd, &(message_header) {.type = GET_LISTS, .size = sizeof(message_header)});

    while (state->keep_running) {
        process_api_messages(state->socket_fd);
        process_user_input(state);
        render(state);

        if (!is_socket_alive(state->socket_fd))
            state->connection_status = DISCONNECTED;

        if (state->connection_status == DISCONNECTED)
            state->keep_running = false;

        nanosleep((const struct timespec[]){{0, 1000000000/60}}, NULL);
    }
}

void client_stop_gui() {
    tcsetattr(0, TCSANOW, &terminal_settings_original);
    show_cursor();
    printf("\n");
    state->keep_running = false;
    while (state->lists != NULL)
        client_delete_lists(state);
}

void client_connection_fail_listener() {
    if (state != NULL)
        state->connection_status = DISCONNECTED;
    client_stop_service();
}

void client_stop_interrupt_listener(__attribute__((unused)) int signum) {
    client_stop_service();
}

void client_dispose_state() {
    free(state);
}