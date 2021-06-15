#pragma once

#include <client/client_state.h>

#define INPUT_TAB               9
#define INPUT_ENTER             13
#define INPUT_ESCAPE            27
#define INPUT_BACKSPACE         127
#define INPUT_CTRL_BACKSPACE    8

#define INPUT_CTRL_A 1
#define INPUT_CTRL_B 2
#define INPUT_CTRL_D 4
#define INPUT_CTRL_E 5
#define INPUT_CTRL_F 6
#define INPUT_CTRL_G 7
#define INPUT_CTRL_H 8
#define INPUT_CTRL_J 10
#define INPUT_CTRL_K 11
#define INPUT_CTRL_L 12
#define INPUT_CTRL_N 14
#define INPUT_CTRL_O 15
#define INPUT_CTRL_P 16
#define INPUT_CTRL_Q 17
#define INPUT_CTRL_R 18
#define INPUT_CTRL_S 19
#define INPUT_CTRL_T 20
#define INPUT_CTRL_U 21
#define INPUT_CTRL_W 23
#define INPUT_CTRL_X 24
#define INPUT_CTRL_Y 25
#define INPUT_CTRL_Z 26

#define INPUT_SLASH 47

#define INPUT_OPEN_BRACKET  91

#define INPUT_ARROWS_END_UP     65
#define INPUT_ARROWS_END_DOWN   66
#define INPUT_ARROWS_END_RIGHT  67
#define INPUT_ARROWS_END_LEFT   68
#define INPUT_SHIFT_TAB_END     90

typedef void (*key_listener_function)(int key, client_state* state);

void add_key_listener(int key, key_listener_function function);
void add_any_key_listener(key_listener_function function);
bool process_user_input(client_state* state);
void process_key(client_state* state, int key);
void get_input(client_state* state, size_t max_length, void (*input_listener)(client_state*), char* prompt);
