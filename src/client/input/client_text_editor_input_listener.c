#include <client/input/client_text_editor_input_input.h>

#include <string.h>

void add_char(char* string, char new_char, size_t index) {
    for (size_t i = strlen(string); i > index; i--)
        string[i + 1] = string[i];
    string[index] = new_char;
}

void remove_char(char* string, size_t index) {
    for (size_t i = index + 1; i < strlen(string); i++)
        string[i - 1] = string[i];
    string[strlen(string) - 1] = 0;
}

void client_text_editor_input_listener(int key, client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_EDIT) {
        if (key >= 32 && key <= 126) {
            if (strlen(state->buffer) < state->buffer_limit) {
                add_char(state->buffer, (char) key, state->caret_position);
                state->caret_position++;
            }
        } else if (key == INPUT_BACKSPACE) {
            if (strlen(state->buffer) > 0 && state->caret_position > 0) {
                remove_char(state->buffer, state->caret_position - 1);
                state->caret_position--;
            }
        } else if (key == INPUT_CTRL_BACKSPACE) {
            while (strlen(state->buffer) > 0 && state->buffer[state->caret_position - 1] == ' ') {
                remove_char(state->buffer, state->caret_position - 1);
                state->caret_position--;
            }
            while (strlen(state->buffer) > 0 && state->buffer[state->caret_position - 1] != ' ') {
                remove_char(state->buffer, state->caret_position - 1);
                state->caret_position--;
            }
        } else if (key == INPUT_CTRL_L) {
            while (strlen(state->buffer) > 0)
                remove_char(state->buffer, strlen(state->buffer) - 1);
            state->caret_position = 0;
        }
    }
}
