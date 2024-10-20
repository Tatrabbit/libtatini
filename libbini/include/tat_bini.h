#ifndef TAT_BINI_HPP
#define TAT_BINI_HPP

#include "stddef.h"

// TODO use TAT_ERR_*
enum {
    BINI_ERR_SUCCESS = 0,

    BINI_ERR_MEMORY,
    BINI_ERR_FILE,
    BINI_ERR_STATE,
    BINI_ERR_COUNT,
};

typedef struct tat_mempool_s tat_mempool_t;

typedef struct bini_kv_s {
    const char *key;
    const char *value;
    struct bini_kv_s *next; // TODO hide this one in an opaque, inner struct
} bini_kv_t;

typedef struct {
    const char *name;

    size_t key_count;
    bini_kv_t *keys;
    bini_kv_t *last_key;
} bini_section_t;

struct bini_chunk_datafile_s {
    int type;
    void *memory_pool;

    unsigned int section_count;
    bini_section_t **sections;
};

// TODO remove
typedef union {
    int type;
    struct bini_chunk_datafile_s datafile;
} bini_chunk_t;

typedef struct {
    int type;
    tat_mempool_t *mempool;

    size_t chunk_count;
    bini_chunk_t **chunks;
    bini_chunk_t owned_chunks[];
} bini_op_t;

// TODO opaque pointer
typedef struct bini_files_s {
    size_t count;
    union bini_files_u *files;
} bini_files_t;

// typedef int (*bini_error_handler_t)(int err, const char *fmt, ...);
//
// #ifndef TAT_BINI_C
// extern bini_error_handler_t bini_error_handler;
// #endif
struct bini_textfile_info_s;

//     * Each state holds allocated strings for when key/value pairs are added or modified.
// * Depending on your use case, you may choose to create one state for all files, or associate
// * a state with a specific file or set of files. Use one global state if your use case is "one and done"
// * AKA you need to parse a file or a set of them, and then no more usage is needed.


// * \note Added parameters, created strings, etc. are only valid while this object exists.
// *       To prevent memory leaks, \ref bini_free() must be called.
bini_op_t *bini_parse_multi(tat_mempool_t *mempool, const bini_files_t *files);

void bini_free_ops(bini_op_t *ops);

bini_section_t *bini_find_section(const bini_chunk_t *chunk, const char *name);

bini_section_t *bini_find_section_all(const bini_op_t *ops, const char *name, bini_chunk_t **containing_chunk);

#endif //TAT_BINI_H
