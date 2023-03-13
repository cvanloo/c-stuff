/* cat "/proc/$(pidof a.out)/statm"*/
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


typedef struct tmp_arena tmp_arena;
struct tmp_arena {
    arena *a;
    U64 restore_size;
};

tmp_arena new_scratch(arena *a) {
    tmp_arena t;
    t.a = a;
    t.restore_size = a->size;
    return t;
}

void end_scratch(tmp_arena t) {
    t.a->size = t.restore_size;
}

/* //////////////////// */

typedef struct entity entity;
struct entity {
    entity *next;
    struct {
        U32 x;
        U32 y;
    } position;
    struct {
        U32 x;
        U32 y;
    } velocity;
    /* other data ... */
};

typedef struct game_state game_state;
struct game_state {
    arena *permanent_arena;
    entity *first_free_entity;
};

entity *entity_alloc(game_state *state) {
    entity *result = state->first_free_entity;
    if (result != NULL) {
        state->first_free_entity = state->first_free_entity->next;
        memset(result, 0, sizeof(entity));
    } else {
        result = arena_alloc(state->permanent_arena, sizeof(entity));
    }
    return result;
}

void entity_free(game_state *state, entity* entity) {
    entity->next = state->first_free_entity;
    state->first_free_entity = entity;
}

/* //////////////////// */

int main() {
    size_t alloc_size = GB(16);
    printf("%zu\n", alloc_size);
    arena *a = create_arena(alloc_size);
    printf("%p\n", (void*)a);

    char *str = (char *) arena_alloc(a, 13*sizeof(char));
    memmove(str, "Hello, World!", 13);

    printf("This string was allocated on the arena: %.*s\n", 13, str);

    tmp_arena tmp = new_scratch(a); /* create scratch arena */
    char *longstr = arena_alloc(a, 50*sizeof(char));
    memmove(longstr, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 50);
    printf("This long string is on the scratch arena: %.*s\n", 50, longstr);
    end_scratch(tmp); /* release scratch arena */

    /* /////////////////// */

    game_state game;
    game.permanent_arena = a;

    /* Allocate some entities. */
    int i;
    printf("allocating entities…\n");
    entity *entities[50000];
    for (i = 0; i < 50000; ++i) {
        entities[i] = entity_alloc(&game);
    }
    getchar();
    printf("now freeing some entities…\n");
    /* Free some of them. */
    for (i = 0; i < 15000; ++i) {
        entity* e = entities[i];
        entity_free(&game, e);
    }
    getchar();
    printf("now allocating some more entities…\n");
    /* Allocate more entities. They will reuse the previously freed entities space. */
    for (i = 0; i < 10000; ++i) {
        entities[i] = entity_alloc(&game);
    }

    /* /////////////////// */

    getchar();
    printf("now freeing all memory…\n");

    /* free the entire arena at once */
    free(a);

    /* This would segvault now
     * printf("This string was allocated on the arena: %.*s\n", 13, str);
    */

    getchar();
    printf("bye\n");
}
