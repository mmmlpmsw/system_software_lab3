#pragma once

#include <sys/types.h>

typedef struct task task;
struct task {
    char* name;
    char* description;
    char* list_name;
    time_t initial_time;
    time_t deadline;
    task * next;
};

typedef struct list list;
struct list {
    char * name;
    task * tasks;
    list * next;
};

task* get_task_by_name(list * list, char* name);
task* get_previous_task(list * list, task* t);
task* get_last_task(list * list);