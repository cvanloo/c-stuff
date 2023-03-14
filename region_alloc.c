/* gcc -std=c89 -g region_alloc.c
 * gdb -tui ./a.out
 */
#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct region region;
struct region {
    region *next;
    U64 size;
    U64 capacity;
    byte memory[];
};

typedef struct arena arena;
struct arena {
    region *first;
    region *last;
    U64 count;
};

#define ASSERT(x) do { if (!(x)) { *(int *)0 = 0; } } while(0)

#define KB(n) (n)*(size_t)1024
#define MB(n) (n)*(size_t)1024*1024
#define GB(n) (n)*(size_t)(1024*1024*1024)

#define ALLOC_SIZE KB(16)

arena arena_create() {
    arena a;
    a.first = a.last = NULL;
    a.count = 0;
    return a;
}

region *alloc_region(U64 capacity) {
    region *r = (region *) malloc(sizeof(region) + capacity*sizeof(byte));
    if (r == NULL) return NULL;
    r->capacity = capacity;
    r->size = 0;
    return r;
}

void free_region(region *r) {
    free(r);
}

void *arena_alloc(arena *a, U64 size) {
    if (a->last == NULL) {
        ASSERT(a->first == NULL);
        U64 capacity = ALLOC_SIZE;
        if (capacity < size) {
            capacity = size;
        }
        region *r = alloc_region(capacity);
        a->first = a->last = r;
    }

    while (a->last->size + size > a->last->capacity && a->last->next != NULL) {
        a->last = a->last->next;
    }

    if (a->last->size + size > a->last->capacity) {
        U64 capacity = ALLOC_SIZE;
        if (capacity < size) {
            capacity = size;
        }
        region *r = alloc_region(capacity);
        a->last->next = r;
        a->last = r;
        a->count++;
    }

    void *ptr;
    ptr = &a->last->memory[a->last->size];
    a->last->size += size;

    return ptr;
}

void *arena_realloc(arena *a, void *oldptr, U64 old_size, U64 new_size) {
    if (new_size <= old_size) return oldptr;
    void *newptr = arena_alloc(a, new_size);

    /* copy data over */
    byte *oldptr_byte = oldptr;
    byte *newptr_byte = newptr;
    U64 i;
    for (i = 0; i < old_size; ++i) {
        newptr_byte[i] = oldptr_byte[i];
    }
    return newptr;
}

void arena_reset(arena *a) {
    region *r;
    for (r = a->first; r != NULL; r = r->next) {
        r->size = 0;
    }
    a->last = a->first;
}

void arena_free(arena *a) {
    region *r = a->first;
    while (r) {
        region *r0 = r;
        r = r->next;
        free_region(r0);
    }
    a->first = a->last = NULL;
}

int main() {
    arena a = arena_create();
    char *str = arena_alloc(&a, 13*sizeof(char));
    memmove(str, "Hello, World!", 13);
    printf("%.*s\n", 13, str);
    arena_free(&a);
}
