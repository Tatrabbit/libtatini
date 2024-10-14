#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>

#define MEMCHUNK_SIZE 4096

extern jmp_buf bini_jump_buf;

typedef struct memchunk_s {
    struct memchunk_s *next;
    size_t size;
    char data[];
} bini__memchunk_t;

static bini__memchunk_t *allocated_memchunks = NULL;
static bini__memchunk_t *last_memchunk = NULL;

static bini__memchunk_t *memchunk_new(size_t size) {
    bini__memchunk_t *chunk = malloc(sizeof(bini__memchunk_t) + MEMCHUNK_SIZE);
    if (chunk == NULL)
        longjmp(bini_jump_buf, 1);

    chunk->next = NULL;
    chunk->size = 0;

    if (allocated_memchunks == NULL)
        allocated_memchunks = chunk;

    else if (last_memchunk != NULL)
        last_memchunk->next = chunk;

    return (last_memchunk = chunk);
}

char *bini__memchunk_get(const size_t size) {
    assert(size > 0);
    if (size > MEMCHUNK_SIZE)
        longjmp(bini_jump_buf, 1); // TODO dynamic size

    bini__memchunk_t *chunk = allocated_memchunks;
    if (chunk == NULL)
        chunk = memchunk_new(size);

    for (; chunk != NULL; chunk = chunk->next) {
        const size_t remaining_size = MEMCHUNK_SIZE - chunk->size;
        if (remaining_size <= size)
            goto get_from_chunk;
    }
    chunk = memchunk_new(size);

    char *buf;

get_from_chunk:
    buf = chunk->data + chunk->size;
    chunk->size += size;
    return buf;
}

void bini__memchunk_free() {
    if (allocated_memchunks == NULL)
        return;

    for (bini__memchunk_t *next, *chunk = allocated_memchunks; chunk != NULL; chunk = next) {
        next = chunk->next;
        free(chunk);
    }
    allocated_memchunks = NULL;
    last_memchunk = NULL;
}
