#ifndef TAT_BINI_H
#define TAT_BINI_H

#include <stddef.h>

typedef struct {
    ///@brief filename (optional)
    const char *filename;

    ///@brief Known size of file
    size_t file_size;

    ///@brief Existing buffer containing file contents.
    ///@note This will be modified during parsing
    char *contents;
} bini_textfile_info_t;

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

bini_op_t *bini_parseini_inplace_multi(const bini_textfile_info_t *file, size_t file_count);

bini_section_t *bini_find_section(const bini_chunk_t *chunk, const char *name);

bini_section_t *bini_find_section_all(const bini_op_t *ops, const char *name, bini_chunk_t **containing_chunk);

void bini_free(bini_op_t *ini);

#endif //TAT_BINI_H
