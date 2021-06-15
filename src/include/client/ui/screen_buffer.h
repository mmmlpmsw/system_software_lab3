#pragma once

#include <client/ui/colors.h>
#include <stddef.h>

typedef struct screen_buffer screen_buffer;

screen_buffer* create_buffer(int width, int height);
void dispose_buffer(screen_buffer* buffer);
void screen_set_pixel(screen_buffer* buffer, unsigned short x, unsigned short y, const char* symbol, screen_color color);
char* screen_get_pixel(screen_buffer* buffer, unsigned short x, unsigned short y);
screen_color screen_get_color(screen_buffer* buffer, unsigned short x, unsigned short y);
unsigned short screen_get_buffer_width(screen_buffer* buffer);
unsigned short screen_get_buffer_height(screen_buffer* buffer);
void screen_draw_window(
        screen_buffer* buffer,
        unsigned short x0,
        unsigned short y0,
        unsigned short x1,
        unsigned short y1,
        screen_color color
);
void screen_draw_text(
        screen_buffer* buffer,
        unsigned short x,
        unsigned short y,
        char* text,
        size_t max_length,
        screen_color color
);

void clear_buffer(screen_buffer* buffer);
void screen_set_color(screen_buffer* buffer, unsigned short x, unsigned short y, screen_color color);

int screen_draw_text_box(
        screen_buffer* buffer,
        unsigned short x,
        unsigned short y,
        char* text,
        size_t width,
        size_t max_length,
        screen_color color
);
