#ifndef TAT_BINI_H
#define TAT_BINI_H

#include <stdio.h>
#include "../libbini/bini_types.h"

enum {
    BINI_ERR_SUCCESS = 0,

    BINI_ERR_MEMORY,
    BINI_ERR_FILE,
    BINI_ERR_STATE,
    BINI_ERR_COUNT,
};

typedef struct bini_kv_s {
    const char *key;
    const char *value;
    struct bini_kv_s *next;
} bini_kv_t;

typedef struct {
    const char *name;

    size_t key_count;
    bini_kv_t *keys;
    bini_kv_t *last_key;
} bini_section_t;

struct bini_chunk_datafile_s {
    int type;
    unsigned int section_count;
    bini_section_t **sections;
};

typedef union {
    int type;
    struct bini_chunk_datafile_s datafile;
} bini_chunk_t;

typedef struct {
    size_t chunk_count;
    bini_chunk_t **chunks;
    bini_chunk_t owned_chunks[];
} bini_op_t;

typedef struct bini_files_s {
    size_t count;
    union bini_files_u *files; // TODO opaque pointer
} bini_files_t;

// typedef int (*bini_error_handler_t)(int err, const char *fmt, ...);
//
// #ifndef TAT_BINI_C
// extern bini_error_handler_t bini_error_handler;
// #endif
struct bini_textfile_info_s;

bini_op_t *bini_parse_multi(bini_files_t files);

bini_section_t *bini_find_section(const bini_chunk_t *chunk, const char *name);

bini_section_t *bini_find_section_all(const bini_op_t *ops, const char *name, bini_chunk_t **containing_chunk);

void bini_free(bini_op_t *ops);

#endif //TAT_BINI_H
