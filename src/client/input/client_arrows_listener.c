#include <client/input/client_arrows_listener.h>

#include <stdio.h>
#include <string.h>

void arrow_up(client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_LISTS) {
        state->selected_list--;
        state->selected_task = 0;
        if (state->selected_list <= 0)
            state->selected_list = 0;
    } else if (state->focused_window == CLIENT_WINDOW_TASKS) {
        state->selected_task--;
        if (state->selected_task <= 0)
            state->selected_task = 0;
    } else if (state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION) {
        state->selected_field--;
        if (state->selected_field < 0)
            state->selected_field = 0;
    }
}

void arrow_down(client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_LISTS) {
        if (state->selected_list < count_lists(state)) {
            state->selected_list++;
            state->selected_task = 0;
        }

    } else if (state->focused_window == CLIENT_WINDOW_TASKS) {
        if (state->selected_task < count_tasks(get_list_by_index(state, state->selected_list)))
            state->selected_task++;
    } else if (state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION) {
        if (state->selected_field < 2)
            state->selected_field ++;
    }
}

void arrow_right(client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_LISTS)
        state->focused_window = CLIENT_WINDOW_TASKS;
    else if (state->focused_window == CLIENT_WINDOW_TASKS)
        state->focused_window = CLIENT_WINDOW_TASKS_DESCRIPTION;
    else if (state->focused_window == CLIENT_WINDOW_EDIT) {
        if (state->caret_position < strlen(state->buffer))
            state->caret_position++;
    }
}

void arrow_left(client_state* state) {
    if (state->focused_window == CLIENT_WINDOW_TASKS)
        state->focused_window = CLIENT_WINDOW_LISTS;
    else if (state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION)
        state->focused_window = CLIENT_WINDOW_TASKS;
    else if (state->focused_window == CLIENT_WINDOW_EDIT) {
        if (state->caret_position > 0)
            state->caret_position--;
    }
}

void client_arrows_listener(__attribute__((unused)) int key, client_state* state) {
    int middle_key = getchar();
    int end_key = getchar();

    if (middle_key != INPUT_OPEN_BRACKET) {
        ungetc(end_key, stdin);
        ungetc(middle_key, stdin);
        return;
    }

    switch (end_key) {
        case INPUT_ARROWS_END_UP:
            arrow_up(state);
            break;
        case INPUT_ARROWS_END_DOWN:
            arrow_down(state);
            break;
        case INPUT_ARROWS_END_RIGHT:
            arrow_right(state);
            break;
        case INPUT_ARROWS_END_LEFT:
            arrow_left(state);
            break;
        default:
            ungetc(end_key, stdin);
            ungetc(middle_key, stdin);
    }
}