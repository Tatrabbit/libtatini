#ifndef TAT_LIBTATINI_MEMPOOL_H
#define TAT_LIBTATINI_MEMPOOL_H

#include <stddef.h>


// TODO remove, use TATINI instead
enum {
    TAT_ERR_SUCCESS = 0,

    TAT_ERR_MEMORY,
    TAT_ERR_FILE,
    TAT_ERR_STATE,
    TAT_ERR_COUNT,
};


typedef struct tatini_memchunk_s {
    struct tatini_memchunk_s *next;
    size_t size;
    size_t capacity;
    char data[];
} tat_memchunk_t;


/** \brief Memory pool to reduce malloc() calls.
*/
typedef struct tatini_mempool_s {
    size_t chunk_size;
    tat_memchunk_t *first;
    tat_memchunk_t *last;

    // TODO optimize by skipping all chunks with less than a minimum size
    // tat_memchunk_t *first_available;
} tatini_mempool_t;


// TODO opaque pointer
// typedef void tat_mempool_t;

// typedef struct {
//     const void *const opaque_0;
//     const void *const opaque_1;
//     const size_t opaque_2;
// } tat_mempool_t;


/** \brief Create a new memory pool.
 * \param chunk_size The number of bytes allocated each time malloc() is called.
 * \return Allocated mempool, or NULL if out of memory
 */
tatini_mempool_t *tatini_mempool_new(size_t chunk_size);

void tatini_mempool_free(const tatini_mempool_t *mempool);

void *tatini_mempool_getmem(tatini_mempool_t *mempool, size_t size);

#endif
