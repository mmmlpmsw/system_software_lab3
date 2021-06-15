#include <server/server_state.h>

#include <time.h>
#include <stddef.h>
#include <string.h>

#include <util/mem_util.h>
#include <stdlib.h>
#include <common_state.h>
#include <printf.h>

list* get_list_by_name(server_connected_user* user, char* name) {
    list* current_list = user->lists;
    while (current_list != NULL) {
        if (strcmp(current_list->name, name) == 0) {
            return current_list;
        }
        current_list = current_list->next;
    }
    return NULL;
}

list* get_previous_list(server_connected_user* user, list* l) {
    list* current_list = user->lists;
    while (current_list != NULL && current_list->next != l)
        current_list = current_list->next;
    return current_list;
}

server_connected_user* find_user_by_name(server_state* state, char* name) {
    server_connected_user* current = state->users;
    while (current != NULL && strcmp(current->name, name) != 0)
        current = current->next;
    return current;
}

server_connected_user* find_user_by_fd(server_state* state, int socket_fd) {
    server_connected_user* current = state->users;
    while (current != NULL && current->socket_fd != socket_fd)
        current = current->next;
    return current;
}

void add_user(server_state* state, int socket_fd) {
    server_connected_user* new_user = malloc(sizeof(server_connected_user));
    new_user->next = NULL;
    new_user->socket_fd = socket_fd;
    new_user->name = NULL;
    new_user->lists = NULL;

    if (state->users == NULL) {
        state->users = new_user;
        return;
    }

    server_connected_user* current = state->users;
    server_connected_user* prev = state->users;
    while (current != NULL) {
        prev = current;
        current = current->next;
    }
    prev->next = new_user;
    printf("Added user\n");
}

void remove_user(server_state* state, server_connected_user* user) {
    if (user == NULL)
        return;

    user->socket_fd = -1; // "deleting" user
//    server_connected_user** next_ptr = &state->users;
//    while (*next_ptr != NULL && *next_ptr != user)
//        next_ptr = &(*next_ptr)->next;
//    if (*next_ptr == user)
//        *next_ptr = user->next;
//    free_safe(user->name);
//
//    while (user->lists != NULL)
//        delete_list(user, user->lists->name);

//    free(user);
}

void create_task(server_connected_user *user, char *name, char *description, char *list_name, time_t deadline) {
    task* new_task = malloc(sizeof(task));
    new_task->initial_time = time(NULL);
    new_task->name = strdup(name);
    new_task->deadline = deadline;
    new_task->description = strdup(description);
    new_task->next = NULL;
    new_task->list_name = strdup(list_name);

    list* l = get_list_by_name(user, list_name);

    if (l == NULL)
        return;

    task * t = get_last_task(l);
    if (t == NULL)
        l->tasks = new_task;
    else
        t->next = new_task;

//    fprintf(stderr,"send list %s\n",new_task->list_name);
//    fprintf(stderr,"send task %s\n",new_task->name);
//    fprintf(stderr,"send task description %s\n",new_task->description);
//    fprintf(stderr,"send task init time %ld\n",new_task->initial_time);
//    fprintf(stderr,"send task deadline time %ld\n",new_task->deadline);
}

void update_task(server_connected_user *user, char* old_name, char *new_name, char *description, char *list_name, time_t deadline) {
    list* l = get_list_by_name(user, list_name);
    if (l == NULL)
        return;

    task* t = get_task_by_name(l, old_name);
    if (t == NULL)
        return;
    else {
        t->initial_time = time(NULL);
        t->name = strdup(new_name);
        t->deadline = deadline;
        t->description = strdup(description);
        t->next = NULL;
        t->list_name = strdup(list_name);
    }
}

void delete_task(server_connected_user *user, char *name, char *list_name) {
    list* l = get_list_by_name(user, list_name);
    if (l == NULL)
        return;

    task* t = get_task_by_name(l, name);
    if (t == NULL)
        return;
    else {
        task* prev_task = get_previous_task(l, t);
        if (prev_task == NULL) {
            l->tasks = t->next;
        } else {
            prev_task->next = t->next;
        }
        free(t->name);
        free(t->list_name);
        free(t->description);
        free(t);
    }
}

void create_list(server_connected_user *user, char *name) {
    list* new_list = malloc(sizeof (list));
    new_list->name = strdup(name);
    new_list->next = NULL;
    new_list->tasks = NULL;

    if (user->lists == NULL)
        user->lists = new_list;
    else {
        list * current = user->lists;
        while (current->next != NULL)
            current = current->next;
        current->next = new_list;
    }
}

void delete_list(server_connected_user *user, char *name) {
    list* list_for_delete = get_list_by_name(user, name);
    if (list_for_delete == NULL)
        return;
    else {
        list* prev_list = get_previous_list(user, list_for_delete);
        if (prev_list == NULL)
            user->lists = list_for_delete->next;
        else
            prev_list->next = list_for_delete->next;
        while (list_for_delete->tasks != NULL) {
            delete_task(user, list_for_delete->tasks->name, name);
        }
        free_safe(list_for_delete->name);
        free(list_for_delete);
    }
}