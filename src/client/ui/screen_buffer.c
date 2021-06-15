#include <client/ui/screen_buffer.h>

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

struct screen_buffer {
    unsigned short width;
    unsigned short height;
    uint64_t* symbols;
    char** colors;
};

screen_buffer* create_buffer(int width, int height) {
    screen_buffer* buffer = malloc(sizeof(screen_buffer));
    buffer->width = width;
    buffer->height = height;
    if (width == 0)
        buffer->width = 80;
    if (height == 0)
        buffer->height = 24;
    buffer->symbols = malloc(sizeof(uint64_t)*width*height);
    buffer->colors = malloc(sizeof(char*)*width*height);
    memset(buffer->symbols, 0, sizeof(uint64_t)*width*height);

    clear_buffer(buffer);
    return buffer;
}

void clear_buffer(screen_buffer* buffer) {
    for (size_t y = 0; y < screen_get_buffer_height(buffer); y++) {
        for (size_t x = 0; x < screen_get_buffer_width(buffer); x++) {
            screen_set_pixel(buffer, x, y, " ", COLOR_RESET);
        }
    }
}

void dispose_buffer(screen_buffer* buffer) {
    if (buffer == NULL)
        return;
    free(buffer->symbols);
    free(buffer->colors);
    free(buffer);
}

void screen_set_pixel(screen_buffer* buffer, unsigned short x, unsigned short y, const char* symbol, screen_color color) {
    if (x >= buffer->width || y >= buffer->height)
        return;
    uint64_t symbol_safe = 0;
    memcpy(&symbol_safe, symbol, strlen(symbol));
    buffer->symbols[buffer->width*y + x] = symbol_safe;
    buffer->colors[buffer->width*y + x] = color;
}

char* screen_get_pixel(screen_buffer* buffer, unsigned short x, unsigned short y) {
    if (x >= buffer->width || y >= buffer->height)
        return " ";
    return (char*) &buffer->symbols[buffer->width*y + x];
}

screen_color screen_get_color(screen_buffer* buffer, unsigned short x, unsigned short y) {
    if (x >= buffer->width || y >= buffer->height)
        return "";
    return buffer->colors[buffer->width*y + x];
}

unsigned short screen_get_buffer_width(screen_buffer* buffer) {
    return buffer->width;
}

unsigned short screen_get_buffer_height(screen_buffer* buffer) {
    return buffer->height;
}


void screen_draw_line_vertical(
        screen_buffer* buffer,
        unsigned short x,
        unsigned short y0,
        unsigned short y1,
        char* symbol,
        screen_color color
) {
    for (unsigned short y = y0; y <= y1; y++) {
        screen_set_pixel(buffer, x, y, symbol, color);
    }
}

void screen_draw_line_horizontal(
        screen_buffer* buffer,
        unsigned short y,
        unsigned short x0,
        unsigned short x1,
        char* symbol,
        screen_color color
) {
    for (unsigned short x = x0; x <= x1; x++) {
        screen_set_pixel(buffer, x, y, symbol, color);
    }
}

void screen_draw_window(
        screen_buffer* buffer,
        unsigned short x0,
        unsigned short y0,
        unsigned short x1,
        unsigned short y1,
        screen_color color
) {
    screen_draw_line_horizontal(buffer, y0, x0 + 1, x1 - 1, "\u2500", color);
    screen_draw_line_horizontal(buffer, y1, x0 + 1, x1 - 1, "\u2500", color);
    screen_draw_line_vertical(buffer, x0, y0 + 1, y1 - 1, "\u2502", color);
    screen_draw_line_vertical(buffer, x1, y0 + 1, y1 - 1, "\u2502", color);
    screen_set_pixel(buffer, x0, y0, "\u250C", color);
    screen_set_pixel(buffer, x1, y0, "\u2510", color);
    screen_set_pixel(buffer, x0, y1, "\u2514", color);
    screen_set_pixel(buffer, x1, y1, "\u2518", color);
}

void screen_draw_text(
        screen_buffer* buffer,
        unsigned short x,
        unsigned short y,
        char* text,
        size_t max_length,
        screen_color color
) {
    for (size_t i = 0; i < strlen(text); i++) {
        char symbol[2] = {0};
        symbol[0] = text[i];
        screen_set_pixel(buffer, x + i, y, symbol, color);
        if (max_length > 0 && max_length < strlen(text) && max_length - i < 3)
            screen_set_pixel(buffer, x + i, y, ".", color);
        if (max_length > 0 && i >= max_length)
            break;
    }
}

int next_word_length(const char* string) {
    int len = 0;
    while (string[len] != ' ' && string[len] != '\0')
        len++;
    return len;
}

void screen_set_color(screen_buffer* buffer, unsigned short x, unsigned short y, screen_color color) {
    if (x >= buffer->width || y >= buffer->height)
        return;
    buffer->colors[buffer->width*y + x] = color;
}

int screen_draw_text_box(
        screen_buffer* buffer,
        unsigned short x,
        unsigned short y,
        char* text,
        size_t width,
        size_t max_length,
        screen_color color
) {
    unsigned short caret_x = x;
    unsigned short caret_y = y;
    for (size_t i = 0; i < strlen(text); i++) {
        char symbol[2] = {0};
        symbol[0] = text[i];
        screen_set_pixel(buffer, caret_x, caret_y, symbol, color);
        caret_x++;
        if (
                caret_x - x >= width ||
                (
                        i < strlen(text) - 1 &&
                        text[i] == ' ' &&
                        text[i+1] != ' ' &&
                        next_word_length(&text[i + 1]) >= width - (caret_x - x)
                )
                ) {
            caret_y++;
            caret_x = x;
        }
        if (max_length > 0 && max_length < strlen(text) && max_length - i < 3)
            screen_set_pixel(buffer, x + i, y, ".", color);
        if (max_length > 0 && i >= max_length)
            break;
    }
    return caret_y - y;
}
