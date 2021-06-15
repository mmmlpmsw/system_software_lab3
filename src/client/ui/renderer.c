#include <client/ui/renderer.h>

#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include <client/ui/screen_buffer.h>

typedef unsigned short ushort;

ushort  old_width = 0,
        old_height = 0;

struct winsize window_size;

screen_buffer* buffer;

void update_buffer() {
    ioctl(0, TIOCGWINSZ, &window_size); // Get size of terminal
    ushort width = window_size.ws_col;
    ushort height = window_size.ws_row;
    if (buffer == NULL || old_width != width || old_height != height) { // Size changed, recreate buffer
        dispose_buffer(buffer);
        buffer = create_buffer(width, height);
        old_width = screen_get_buffer_width(buffer);
        old_height = screen_get_buffer_height(buffer);
    }
}

void print_buffer() {
    ssize_t colored_pixel_size = 2*MAX_COLOR_SIZE + sizeof(uint64_t) + 1;
    ssize_t stdout_buffer_size = (colored_pixel_size * screen_get_buffer_width(buffer) + 10) * (screen_get_buffer_height(buffer) + 1) + 1;
    char* stdout_buffer = malloc(stdout_buffer_size);
    char* pixel_buffer = malloc(colored_pixel_size);
    memset(stdout_buffer, 0, stdout_buffer_size);

    for (ushort y = 0; y < screen_get_buffer_height(buffer); y++) {
        sprintf(pixel_buffer, "\033[%d;0f", y);
        strcat(stdout_buffer, pixel_buffer);
        for (ushort x = 0; x < screen_get_buffer_width(buffer); x++) {
            sprintf(pixel_buffer, "%s%s%s", screen_get_color(buffer, x, y), screen_get_pixel(buffer, x, y), COLOR_RESET);
            strcat(stdout_buffer, pixel_buffer);
        }
    }

    sprintf(pixel_buffer, "\033[%d;0f", screen_get_buffer_height(buffer));
    strcat(stdout_buffer, pixel_buffer);
    for (ushort x = 0; x < screen_get_buffer_width(buffer); x++) // To prevent garbage in bottom line
        strcat(stdout_buffer, " ");

    stdout_buffer[stdout_buffer_size - 1] = 0;
    printf("%s", stdout_buffer);
    fflush(stdout);

    free(stdout_buffer);
    free(pixel_buffer);
}

void render_lists(client_state* state, ushort x0, ushort y0, ushort x1, ushort y1) {
    screen_color border_color = COLOR_WINDOW_BORDER(state->focused_window == CLIENT_WINDOW_LISTS);
    screen_draw_window(buffer, x0, y0, x1, y1, border_color);
    screen_draw_text(buffer, x0 + 1, y0 + 1, "Lists", 0, COLOR_TEXT_GREY);
    list * current = state->lists;
    int offset = 0;
    while (current != NULL) {
        screen_draw_text(buffer, x0 + 1, y0 + 2 + offset, state->selected_list == offset ? ">" : " ", 0, COLOR_TEXT_DEFAULT);
        screen_draw_text(buffer, x0 + 2, y0 + 2 + offset, current->name, x1 - x0 - 1, COLOR_TEXT_DEFAULT);
//        screen_color name_color = state->selected_list == offset ? COLOR_TEXT_HIGHLIGHTED_BLACK : COLOR_TEXT_DEFAULT;
//        screen_draw_text(buffer, x0 + 1, y0 + 1 + offset, current->name, x1-x0-2, name_color);
        current = current->next;
        offset ++;
    }
}

void render_tasks(client_state* state, ushort x0, ushort y0, ushort x1, ushort y1) {
    screen_color border_color = COLOR_WINDOW_BORDER(state->focused_window == CLIENT_WINDOW_TASKS);
    screen_draw_window(buffer, x0, y0, x1, y1, border_color);
    screen_draw_text(buffer, x0 + 1, y0 + 1, "Tasks", 0, COLOR_TEXT_GREY);
    list* current_list = state->lists;
    for (int i = 0; current_list != NULL && i < state->selected_list; i++) {
        current_list = current_list->next;
    }
    int offset = 0;
    if (current_list != NULL) {
        task* current_task = current_list->tasks;
        while (current_task != NULL) {
            screen_draw_text(buffer, x0 + 1, y0 + 2 + offset, state->selected_task == offset ? ">" : " ", 0, COLOR_TEXT_DEFAULT);
            screen_draw_text(buffer, x0 + 2, y0 + 2 + offset, current_task->name, x1 - x0 - 1, COLOR_TEXT_DEFAULT);
            current_task = current_task->next;
            offset++;
        }
    }
//    fprintf(stderr,"%d",offset);
}

void render_description(client_state* state, ushort x0, ushort y0, ushort x1, ushort y1) {
    screen_color border_color = COLOR_WINDOW_BORDER(state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION);
    screen_draw_window(buffer, x0, y0, x1, y1, border_color);
    screen_draw_text(buffer, x0 + 1, y0 + 1, "Task info", 0, COLOR_TEXT_GREY);

    if (state->focused_window == CLIENT_WINDOW_TASKS || state->focused_window == CLIENT_WINDOW_TASKS_DESCRIPTION) {
        task* current_task = get_task_by_index(get_list_by_index(state, state->selected_list), state->selected_task);
        screen_draw_text(buffer, x0 + 2, y0 + 2, "Name", 0, COLOR_TEXT_DEFAULT);
        if (current_task != NULL)
            screen_draw_text(buffer, x0 + 2, y0 + 3, current_task->name,0, COLOR_TEXT_DEFAULT);
        if (state->selected_field == 0)
            screen_draw_text(buffer, x0 + 1, y0 + 3, ">", 0, COLOR_TEXT_DEFAULT);

        screen_draw_text(buffer, x0 + 2, y0 + 5, "Initial time", 0, COLOR_TEXT_DEFAULT);
        if (current_task != NULL) {
            char date_buffer[128] = {0};
            strftime(date_buffer, 128, "%H:%M:%S %d.%m.%Y", localtime(&current_task->initial_time));
            screen_draw_text(buffer, x0 + 2, y0 + 6, date_buffer, 0, COLOR_TEXT_DEFAULT);
        }

        screen_draw_text(buffer, x0 + 2, y0 + 8, "Description", 0, COLOR_TEXT_DEFAULT);
        if (current_task != NULL)
            screen_draw_text_box(buffer, x0 + 2, y0 + 9, current_task->description, x1 - x0 - 2, 0, COLOR_TEXT_DEFAULT);
        if (state->selected_field == 1)
            screen_draw_text(buffer, x0 + 1, y0 + 9, ">", 0, COLOR_TEXT_DEFAULT);

        screen_draw_text(buffer, x0 + 2, y0 + 14, "Deadline", 0, COLOR_TEXT_DEFAULT);
        if (current_task != NULL) {
            char date_buffer[128] = {0};
            strftime(date_buffer, 128, "%H:%M:%S %d.%m.%Y", localtime(&current_task->deadline));
            screen_draw_text(buffer, x0 + 2, y0 + 15, date_buffer, 0, COLOR_TEXT_DEFAULT);
        }
        if (state->selected_field == 2)
            screen_draw_text(buffer, x0 + 1, y0 + 15, ">", 0, COLOR_TEXT_DEFAULT);
    }

}

void render_editor(client_state* state, ushort x0, ushort y0, ushort x1, ushort y1) {
    screen_color border_color = COLOR_WINDOW_BORDER(state->focused_window == CLIENT_WINDOW_EDIT);
    screen_draw_text(buffer, x0 + 1, y0 + 1, "Edit", 0, COLOR_TEXT_GREY);
    if (state->prompt != NULL)
        screen_draw_text(buffer, x0 + 1, y0 + 2, state->prompt, 0, COLOR_TEXT_DEFAULT);
    screen_draw_window(buffer, x0, y0, x1, y1, border_color);
    if (state->buffer != NULL) {
        screen_draw_text(buffer, x0 + 1, y0 + 4, state->buffer, x1 - x0 - 2, COLOR_TEXT_DEFAULT);
        screen_set_color(buffer, x0 + 1 + state->caret_position, y0 + 4, COLOR_BLINKING_HIGHLIGHTED);
    }
}

void render(client_state* state) {
    update_buffer();
    clear_buffer(buffer);

    ushort w = screen_get_buffer_width(buffer);
    ushort h = screen_get_buffer_height(buffer);

    render_lists(state, 1, 1, w/4, 2*h/3);
    render_tasks(state, w/4 + 1, 1, w/2, 2*h/3);
    render_description(state, w/2 + 1, 1, w-1, 2*h/3);
    render_editor(state, 1, 2*h/3 + 1, w-1, h - 1);

    print_buffer();
}

void hide_cursor() {
    fputs("\033[?25l", stdout);
}

void show_cursor() {
    fputs("\033[?25h", stdout);
}