#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KB(n) (n)*(size_t)1024
#define MB(n) (n)*(size_t)1024*1024
#define GB(n) (n)*(size_t)(1024*1024*1024)

typedef struct arena arena;
struct arena {
    U64 size;
    U64 capacity;
    byte memory[];
};

arena *create_arena(U64 capacity) {
    arena *a = (arena *) malloc(sizeof(arena) + capacity*sizeof(byte));
    if (a == NULL) return NULL;
    a->capacity = capacity;
    a->size = 0;
    return a;
}

void *arena_alloc(arena *a, U64 size) {
    U64 alloc_pos = a->size;
    U64 capacity = a->capacity;
    U64 new_pos = alloc_pos + size;

    if (new_pos < capacity) {
        void *ptr = a->memory + alloc_pos;
        a->size = new_pos;
        return ptr;
    }
    return NULL;

}

void arena_free(arena *a, U64 size) {
    U64 alloc_pos = a->size;
    a->size = alloc_pos - size;
    if (a->size < 0) {
        a->size = 0;
    }
}


int main() {
    size_t alloc_size = GB(16);
    printf("%zu\n", alloc_size);
    arena *a = create_arena(alloc_size);
    printf("%p\n", (void*)a);

    char* str = (char *) arena_alloc(a, 13*sizeof(char));
    memmove(str, "Hello, World!", 13);

    printf("This string was allocated on the arena: %.*s\n", 13, str);

    sleep(10); /* 10 seconds */
    printf("now freeing all memory…\n");

    /* free the entire arena at once */
    free(a);

    /* This would segvault now
     * printf("This string was allocated on the arena: %.*s\n", 13, str);
    */

    do {} while (1);
}
