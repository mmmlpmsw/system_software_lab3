#include <util/mem_util.h>

#include <stddef.h>
#include <stdlib.h>

void free_safe(void* ptr) {
    if (ptr != NULL)
        free(ptr);
}