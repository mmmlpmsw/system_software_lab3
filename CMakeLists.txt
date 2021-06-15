cmake_minimum_required(VERSION 3.16)
project(app C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fsanitize=address")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g3")
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
        src/main.c
        src/client/client.c
        src/client/ui/renderer.c
        src/server/server.c
        src/server/server_logic.c
        src/client/client_logic.c
        src/api.c
        src/server/server_state.c
        src/client/input/client_input.c
        src/client/ui/screen_buffer.c
        src/client/input/client_arrows_listener.c
        src/client/input/client_tab_listener.c
        src/util/mem_util.c
        src/client/client_state.c
        src/common_state.c
        src/client/input/client_add_listener.c
        src/client/input/client_text_editor_input_listener.c
        src/client/input/client_enter_listener.c
        src/client/input/client_delete_listener.c
        src/client/input/client_edit_listener.c)

set(HEADERS
        src/include/client/client_state.h
        src/include/client/ui/renderer.h
        src/include/api.h
        src/include/server/server.h
        src/include/server/server_state.h
        src/include/server/server_logic.h
        src/include/client/client_logic.h
        src/include/client/ui/screen_buffer
        src/include/client/input/client_input
        src/include/constraints
        src/include/client/ui/colors
        src/include/client/input/client_arrows_listener
        src/include/client/input/client_tab_listener
        src/include/util/mem_util
        src/include/common_state
        src/include/client/input/client_add_listener.h
        src/include/client/input/client_text_editor_input_input.h
        src/include/client/input/client_enter_listener.h
        src/include/client/input/client_delete_listener.h src/include/client/input/client_edit_listener.h)

include_directories(${PROJECT_SOURCE_DIR}/src/command)
include_directories(${PROJECT_SOURCE_DIR}/src/filesystem)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/util)

add_executable(app ${SOURCES} ${HEADERS})