#ifndef TAT_LIBTATINI_H
#define TAT_LIBTATINI_H

#include <stdio.h>

enum {
    TATINI_ERR_SUCCESS = 0,

    TATINI_ERR_MEMORY,
    TATINI_ERR_FILE,
    TATINI_ERR_STATE,
    TATINI_ERR_COUNT,
};

typedef struct tatini_mempool_s tatini_mempool_t;
typedef struct tatini_infos_s tatini_infos_t;

// TODO hide this one in an opaque, inner struct
typedef struct tatini_kv_ref_s {
    const char *key;
    const char *value;
    struct tatini_kv_s *next;
} tatini_kv_ref_t;

typedef struct tatini_section_ref_s {
    const char *name;

    size_t key_count;
    tatini_kv_ref_t *keys;
    tatini_kv_ref_t *last_key;
} tatini_section_ref_t;

typedef struct tatini_chunk_s {
    const char *name;

    size_t n_sections;
    tatini_section_ref_t *sections;
} tatini_chunk_t;

typedef struct tatini_file_s {
    const char *name;

    size_t n_sections;
    tatini_section_ref_t *sections;

    /// \brief The size of the buffer
    size_t size;

    /// \brief Existing buffer containing file contents.
    /// \note This will be modified during parsing
    char *contents;

    FILE *handle;
} tatini_file_t;

typedef struct {
    size_t n_chunks;
    tatini_chunk_t **chunks;
} tatini_state_t;

// typedef int (*bini_error_handler_t)(int err, const char *fmt, ...);
//
// #ifndef TAT_LIBTATINI_C
// extern tatini_error_handler_t bini_error_handler;
// #endif

void tatini_state_free(tatini_state_t *state);

void tatini_parse_inplace(tatini_mempool_t *mempool, tatini_file_t *file);

tatini_section_ref_t *tatini_section_find_first(const tatini_state_t *state, const char *name);

#endif