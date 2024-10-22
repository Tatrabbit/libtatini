#include "./include/tat/libtatini_mempool.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline void init_chunk(tat_memchunk_t *chunk, size_t capacity) {
    *chunk = (tat_memchunk_t){
        .
        next = NULL,
        .
        size = 0,
        .
        capacity = capacity
    };
}

static tat_memchunk_t *alloc_chunk(tatini_mempool_t *mempool, size_t capacity) {
    tat_memchunk_t *chunk = malloc(sizeof(tat_memchunk_t) + mempool->chunk_size);
    if (!chunk)
        return NULL; // TODO error handling
    // longjmp(tatini_jump_buf, 1);

    init_chunk(chunk, capacity);

    mempool->last->next = chunk;
    mempool->last = chunk;

    return chunk;
}

tatini_mempool_t *tatini_mempool_new(const size_t chunk_size) {
    // Assert that the first memchunk is correctly aligned.
    _Static_assert((sizeof(tatini_mempool_t) % _Alignof(tat_memchunk_t)) == 0);

    // Allocate two for the price of one
    const size_t first_chunk_size = sizeof(tat_memchunk_t) + chunk_size;
    tatini_mempool_t *mempool = malloc(sizeof(tatini_mempool_t) + first_chunk_size);
    if (!mempool)
        return NULL; // TODO error handling

    tat_memchunk_t *const chunk = (tat_memchunk_t *) (mempool + 1);

    *mempool = (tatini_mempool_t){
        .
        chunk_size = chunk_size,
        .
        first = chunk,
        .
        last = chunk
    };

    return mempool;
}

void tatini_mempool_free(const tatini_mempool_t *mempool) {
    tat_memchunk_t *first = mempool->first;

    if (!first)
        return;

    // TODO fix this

    for (tat_memchunk_t *next, *chunk = first; chunk; chunk = next) {
        next = chunk->next;
        free(chunk);
    }
}

void *tatini_mempool_getmem(tatini_mempool_t *mempool, const size_t size) {
    const register size_t chunk_size = mempool->chunk_size;

    tat_memchunk_t *chunk;
    if (size > chunk_size) {
        chunk = alloc_chunk(mempool, size);
        goto test_chunk;
    }

    for (chunk = mempool->first; chunk; chunk = chunk->next) {
        assert(chunk->capacity >= chunk->size);

        const size_t size_remaining = chunk->capacity - chunk->size;
        if (size_remaining >= size)
            goto assign_memory;
    }

    chunk = alloc_chunk(mempool, chunk_size);

test_chunk:
    if (!chunk) {
        return NULL; // TODO error handling
        // longjmp(bini_jump_buf, 1);
    }

    // ReSharper disable once CppJoinDeclarationAndAssignment
    char *buf;

assign_memory:
    buf = chunk->data + chunk->size;
    chunk->size += size;
    return buf;
}
