#include <common_state.h>

#include <stddef.h>
#include <string.h>

task* get_task_by_name(list * list, char* name) {
    task * current_task = list->tasks;
    while (current_task != NULL) {
        if (strcmp(current_task->name, name) == 0) {
            return current_task;
        }
        current_task = current_task->next;
    }
    return NULL;
}

task* get_previous_task(list * list, task* t) {
    task* current_task = list->tasks;
    while (current_task != NULL && current_task->next != t)
        current_task = current_task->next;
    return current_task;
}

task* get_last_task(list * list) {
    task * current_task = list->tasks;
    while (current_task != NULL && current_task->next != NULL) {
        current_task = current_task->next;
    }
    return current_task;
}