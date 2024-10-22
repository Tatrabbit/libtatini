#ifndef TAT_LIBTATINI_H
#define TAT_LIBTATINI_H

#include <stddef.h>

enum {
    TATINI_ERR_SUCCESS = 0,

    TATINI_ERR_MEMORY,
    TATINI_ERR_FILE,
    TATINI_ERR_STATE,
    TATINI_ERR_COUNT,
};

typedef struct tatini_mempool_s tatini_mempool_t;

typedef struct tatini_kv_s {
    const char *key;
    const char *value;
    struct tatini_kv_s *next; // TODO hide this one in an opaque, inner struct
} tatini_kv_t;

typedef struct {
    const char *name;

    size_t key_count;
    tatini_kv_t *keys;
    tatini_kv_t *last_key;
} tatini_section_t;

struct tatini_chunk_datafile_s {
    int type;
    void *memory_pool;

    unsigned int section_count;
    tatini_section_t **sections;
};

// TODO remove
typedef union {
    int type;
    struct tatini_chunk_datafile_s datafile;
} tatini_chunk_t;

typedef struct {
    int type;
    tatini_mempool_t *mempool;

    size_t chunk_count;
    tatini_chunk_t **chunks;
    tatini_chunk_t owned_chunks[];
} tatini_op_t;

// TODO opaque pointer
typedef struct tatini_files_s {
    size_t count;
    union tatini_files_u *files;
} tatini_files_t;

// typedef int (*bini_error_handler_t)(int err, const char *fmt, ...);
//
// #ifndef TAT_LIBTATINI_C
// extern tatini_error_handler_t bini_error_handler;
// #endif

//     * Each state holds allocated strings for when key/value pairs are added or modified.
// * Depending on your use case, you may choose to create one state for all files, or associate
// * a state with a specific file or set of files. Use one global state if your use case is "one and done"
// * AKA you need to parse a file or a set of them, and then no more usage is needed.


// * \note Added parameters, created strings, etc. are only valid while this object exists.
// *       To prevent memory leaks, \ref bini_free() must be called.
tatini_op_t *tatini_parse_multi(tatini_mempool_t *mempool, const tatini_files_t *files);

void tatini_free_ops(tatini_op_t *ops);

tatini_section_t *tatini_find_section(const tatini_chunk_t *chunk, const char *name);

tatini_section_t *tatini_find_section_all(const tatini_op_t *ops, const char *name, tatini_chunk_t **containing_chunk);

#endif //TAT_BINI_H
