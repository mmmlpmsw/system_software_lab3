#include <client/input/client_tab_listener.h>

#include <stdio.h>

void tab(client_state* state) {
//    if (state->focused_window == CLIENT_WINDOW_SEARCH)
//        state->focused_window = CLIENT_WINDOW_BOOKS_LIST;
//    else if (state->focused_window == CLIENT_WINDOW_BOOKS_LIST)
//        state->focused_window = CLIENT_WINDOW_BOOK_INFO;
//    else
//        state->focused_window = CLIENT_WINDOW_SEARCH;
}

void shift_tab(client_state* state) {
//    if (state->focused_window == CLIENT_WINDOW_SEARCH)
//        state->focused_window = CLIENT_WINDOW_BOOK_INFO;
//    else if (state->focused_window == CLIENT_WINDOW_BOOKS_LIST)
//        state->focused_window = CLIENT_WINDOW_SEARCH;
//    else
//        state->focused_window = CLIENT_WINDOW_BOOKS_LIST;
}

void client_tab_listener(int key, client_state* state) {
    if (key == INPUT_TAB) {
        tab(state);
        return;
    }

//    int middle_key = getchar();
//    int end_key = getchar();
//
//    if (middle_key != INPUT_OPEN_BRACKET || end_key != INPUT_SHIFT_TAB_END) {
//        ungetc(end_key, stdin);
//        ungetc(middle_key, stdin);
//        return;
//    }
//
//    shift_tab(state);
}