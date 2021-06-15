#include <client/client_state.h>

#include <stddef.h>
#include <string.h>

void client_delete_task(list *l, char *name) {
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
        if (t->description != NULL)
            free(t->description);
        free(t);
    }
}

list* client_get_previous_list(client_state * state, list* l) {
    list* current_list = state->lists;
    while (current_list != NULL && current_list->next != l)
        current_list = current_list->next;
    return current_list;
}

void client_delete_lists(client_state *state) {
    while (state->lists != NULL) {
        list *list_for_delete = state->lists;
        if (list_for_delete == NULL)
            return;
        else {
            list *prev_list = client_get_previous_list(state, list_for_delete);
            if (prev_list == NULL)
                state->lists = list_for_delete->next;
            else
                prev_list->next = list_for_delete->next;
            while (list_for_delete->tasks != NULL) {
                client_delete_task(list_for_delete, list_for_delete->tasks->name);
            }
            free(list_for_delete->name);
            free(list_for_delete);
        }
    }
}

void client_create_list(client_state *state, char *name) {
    list* new_list = malloc(sizeof (list));
    new_list->name = name;
    new_list->next = NULL;
    new_list->tasks = NULL;
    if (state->lists == NULL)
        state->lists = new_list;
    else {
        list * current = state->lists;
        while (current->next != NULL)
            current = current->next;
        current->next = new_list;
    }
}

list* client_get_list_by_name(client_state * state, char* name) {
    list* current_list = state->lists;
    while (current_list != NULL) {
        if (strcmp(current_list->name, name) == 0) {
            return current_list;
        }
        current_list = current_list->next;
    }
    return NULL;
}

void client_create_task(client_state* state, char *name, time_t initial_time, char *description, char *list_name, time_t deadline) {
    task* new_task = malloc(sizeof(task));
    new_task->initial_time = initial_time;
    new_task->name = name;
    new_task->deadline = deadline;
    new_task->description = description;
    new_task->next = NULL;
    new_task->list_name = list_name;

    list* l = client_get_list_by_name(state, list_name);

    if (l == NULL)
        return;

    task * t = get_last_task(l);
    if (t == NULL)
        l->tasks = new_task;
    else
        t->next = new_task;
}

int count_lists(client_state* state) {
    int res = 0;
    list* current = state->lists;
    if (current == NULL) {
        return 0;
    } else {
        while (current->next != NULL) {
            current = current->next;
            res++;
        }
        return res;
    }
}

int count_tasks(list * l) {
    int res = 0;
    task* current = l->tasks;
    if (current == NULL) {
        return 0;
    } else {
        while (current->next != NULL) {
            current = current->next;
            res++;
        }
        return res;
    }
}

list* get_list_by_index(client_state* state, int list_index) {
    list* current_list = state->lists;
    for (int i = 0; current_list != NULL && i < list_index; i++) {
        current_list = current_list->next;
    }
    return current_list;
}

task* get_task_by_index(list* l, int task_index) {
    if (l == NULL || l->tasks == NULL)
        return NULL;
    task* current_task = l->tasks;
    for (int i = 0; i < task_index; i++) {
        current_task = current_task->next;
    }
    return current_task;
}

