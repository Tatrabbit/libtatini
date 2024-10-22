#define TAT_LIBTATINI_C
#include "./include/tat/libtatini.h"

// TODO should be optional!
#include "./include/tat/libtatini_mempool.h"

#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "libtatini_types.h"

#define MEMCHUNK_SIZE 4096

enum {
    CHUNK_NONE = 0,

    CHUNK_DATAFILE_INITIAL = 0x1,
    CHUNK_DATAFILE_OWNED = 0x2,
    CHUNK_DATAFILE_ANY = 0x3,

    CHUNK_OP = 0x4,
};


jmp_buf tatini_jump_buf;

// static int bini_error_handler_default(const int err, const char *fmt, ...) {
//     return err;
// };

// bini_error_handler_t bini_error_handler = bini_error_handler_default;

static char *split_mem_line(char **next, const char *end) {
    char *p = *next;

    // Advance to next actual line
    while (*p == '\n' || *p == '\r')
        p++;

    char *line = p;

    for (; p < end; p++) {
        if (*p == '\n' || *p == '\r') {
            *p = '\0';

            *next = p + 1;
            return line;
        }
    }
    return NULL;
}

typedef struct {
    const char *section;
} parse_state_t;

static const char *parse_line_section3(char *start, char *line, char *end) {
    for (char *p = start; *p != '\0'; p++) {
        switch (*p) {
            case '\0':
            case ';':
            case '#':
                goto success;

            case ' ':
            case '\t':
                continue;

            default:
                return NULL;
        }
    }
success:
    if (end != line)
        *end = '\0';
    return line;
}

static const char *parse_line_section2(char *line) {
    char *start = line, *end = NULL;
    unsigned int has_name = 0;
    int parsing_whitespace = 0;

    for (char *p = line; *p != '\0'; p++) {
        switch (*p) {
            case ']':
                if (end == NULL)
                    end = p;
                return parse_line_section3(p + 1, start, end);
            case ';':
            case '#':
                return NULL;
            case ' ':
            case '\t':
                if (!has_name) {
                    start = p + 1;
                    continue;
                }
                if (!parsing_whitespace) {
                    end = p;
                    parsing_whitespace = 1;
                }
                continue;

            default:
                has_name = 1;
                parsing_whitespace = 0;
                break;
        }
    }
    return NULL;
}

static const char *parse_line_section1(char *line) {
    for (char *p = line; *p != '\0'; p++) {
        switch (*p) {
            case '[':
                return parse_line_section2(p + 1);
            case ' ':
            case '\t':
                continue;
            default:
                return NULL;
        }
    }
    return NULL;
}

static tatini_section_t *new_section(tatini_mempool_t *mempool, const char *name) {
    tatini_section_t *section = tatini_mempool_getmem(mempool, sizeof(tatini_section_t));

    section->name = name;
    section->key_count = 0;
    section->keys = NULL;
    section->last_key = NULL;

    return section;
}

static void parse_file(const tatini_op_t *ops, tatini_chunk_t *chunk, const bini_textfile_info_t *file) {
    char *next = file->contents;
    const char *end = file->contents + file->file_size;

    tatini_section_t *current_section = new_section(ops->mempool, NULL);

    char *line;
    while ((line = split_mem_line(&next, end))) {
        printf("Parsing: %s: \"%s\"", file->filename, line);
        const char *section_name = parse_line_section1(line);

        if (section_name) {
            // current_section = tatini_find_section_all(ops, section_name, NULL);
            current_section = new_section(ops->mempool, section_name);
            printf(" = Section: \"%s\"\n", current_section->name);
        } else
            printf(" (Nothing found.)\n");
    }
}

// int bini_new(bini_t **bini, size_t chunk_size) {
//     tatini_mempool_t *mempool;
//     *bini = mempool = tatini_mempool_new(chunk_size);
//
//     if (!mempool)
//         return TATINI_ERR_MEMORY;
//
//     return TATINI_ERR_SUCCESS;
// }

tatini_section_t *tatini_find_section(const tatini_chunk_t *chunk, const char *name) {
    for (size_t i = 0; i < chunk->datafile.section_count; ++i) {
        tatini_section_t *section = chunk->datafile.sections[i];
        if (strcmp(section->name, name) == 0)
            return section;
    }
    return NULL;
}

tatini_section_t *tatini_find_section_all(const tatini_op_t *ops, const char *name, tatini_chunk_t **containing_chunk) {
    for (size_t i = 0; i < ops->chunk_count; ++i) {
        tatini_chunk_t *chunk = ops->chunks[i];
        if (!(chunk->type & CHUNK_DATAFILE_ANY))
            continue;

        tatini_section_t *section = tatini_find_section(chunk, name);
        if (section != NULL) {
            if (containing_chunk != NULL)
                *containing_chunk = chunk;
            return section;
        }
    }
    return NULL;
}

// TODO move into the multi module
tatini_op_t *tatini_parse_multi(tatini_mempool_t *mempool, const tatini_files_t *files) {
    const size_t files_count = files->count;

    if (files_count == 0)
        return NULL;

    const size_t owned_chunks_size = sizeof(tatini_chunk_t) * files_count;
    const size_t pointer_array_size = sizeof(tatini_chunk_t *) * files_count;
    const size_t buffer_size = sizeof(tatini_op_t) + owned_chunks_size + pointer_array_size;

    char *buf = malloc(buffer_size);
    if (buf == NULL)
        return NULL;

    if (setjmp(bini_jump_buf)) {
        free(buf);
        return NULL;
    }

    tatini_op_t *ops = (tatini_op_t *) buf;
    ops->type = CHUNK_OP;
    ops->mempool = mempool;
    ops->chunk_count = 0;

    const union tatini_files_u *files_array = files->files;

    // pointer array begins after owned chunks.
    ops->chunks = (tatini_chunk_t **) buf + sizeof(tatini_op_t) + owned_chunks_size;
    for (size_t i = 0; i < files_count; i++) {
        tatini_chunk_t *chunk = &ops->owned_chunks[i];
        chunk->type = CHUNK_DATAFILE_INITIAL;
        ops->chunks[i] = chunk;
        ops->chunk_count += 1;

        bini_file_info_base_t *base = (bini_file_info_base_t *) (files_array + i);
        if (base->type == TATINI_INVALID)
            continue;

        parse_file(ops, chunk, (bini_textfile_info_t *) base);
    }

    return ops;
}

void tatini_free_ops(tatini_op_t *ops) {
    free(ops);
}
