#ifndef TAT_MEMORY_POOL_H
#define TAT_MEMORY_POOL_H

#include <stddef.h>


// TODO move to its own file, error handling
enum {
    TAT_ERR_SUCCESS = 0,

    TAT_ERR_MEMORY,
    TAT_ERR_FILE,
    TAT_ERR_STATE,
    TAT_ERR_COUNT,
};


typedef struct tat_memchunk_s {
    struct tat_memchunk_s *next;
    size_t size;
    size_t capacity;
    char data[];
} tat_memchunk_t;


/** \brief Memory pool to reduce malloc() calls.
*/
typedef struct tat_mempool_s {
    size_t chunk_size;
    tat_memchunk_t *first;
    tat_memchunk_t *last;

    // TODO optimize by skipping all chunks with less than a minimum size
    // tat_memchunk_t *first_available;
} tat_mempool_t;


// TODO opaque pointer
// typedef void tat_mempool_t;

// typedef struct {
//     const void *const opaque_0;
//     const void *const opaque_1;
//     const size_t opaque_2;
// } tat_mempool_t;


/** \brief Create a new memory pool.
 *
 * \param[out] bini Allocated object, or NULL on memory error.
 * \param chunk_size The number of bytes allocated each time malloc() is called.
 *
 * \return BINI_ERR_SUCCESS or BINI_ERR_MEMORY
 */
tat_mempool_t *tat_mempool_new(size_t chunk_size);

void tat_mempool_free(const tat_mempool_t *mempool);

void *tat_mempool_getmem(tat_mempool_t *mempool, size_t size);

#endif
