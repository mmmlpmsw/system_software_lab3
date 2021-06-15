#include <client/client_state.h>
#include <client/input/client_enter_listener.h>

void client_enter_listener(int key, client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_EDIT) {
        void (*input_listener)(client_state*) = state->input_listener;
        state->input_listener(state);
        if (input_listener != state->input_listener) {
            //state->caret_position = 0;
            return;
        }
        state->input_listener = NULL;
        free(state->buffer);
        state->buffer = NULL;
        state->caret_position = 0;
        if (state->focused_window == CLIENT_WINDOW_EDIT)
            state->focused_window = CLIENT_WINDOW_LISTS;
        if (state->prompt != NULL) {
            free(state->prompt);
            state->prompt = NULL;
        }

    }
}