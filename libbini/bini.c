#define TAT_BINI_CPP
#include "./include/tat/bini.h"

// TODO should be optional!
#include "./include/tat/mempool.h"

#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "bini_types.h"

#define MEMCHUNK_SIZE 4096

enum {
    CHUNK_NONE = 0,

    CHUNK_DATAFILE_INITIAL = 0x1,
    CHUNK_DATAFILE_OWNED = 0x2,
    CHUNK_DATAFILE_ANY = 0x3,

    CHUNK_OP = 0x4,
};


jmp_buf bini_jump_buf;

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

static bini_section_t *new_section(tat_mempool_t *mempool, const char *name) {
    bini_section_t *section = tat_mempool_getmem(mempool, sizeof(bini_section_t));

    section->name = name;
    section->key_count = 0;
    section->keys = NULL;
    section->last_key = NULL;

    return section;
}

static void parse_file(const bini_op_t *ops, bini_chunk_t *chunk, const bini_textfile_info_t *file) {
    char *next = file->contents;
    const char *end = file->contents + file->file_size;

    bini_section_t *current_section = new_section(ops->mempool, NULL);

    char *line;
    while ((line = split_mem_line(&next, end))) {
        printf("Parsing: %s: \"%s\"", file->filename, line);
        const char *section_name = parse_line_section1(line);

        if (section_name) {
            // current_section = bini_find_section_all(ops, section_name, NULL);
            current_section = new_section(ops->mempool, section_name);
            printf(" = Section: \"%s\"\n", current_section->name);
        } else
            printf(" (Nothing found.)\n");
    }
}

// int bini_new(bini_t **bini, size_t chunk_size) {
//     tat_mempool_t *mempool;
//     *bini = mempool = tat_mempool_new(chunk_size);
//
//     if (!mempool)
//         return BINI_ERR_MEMORY;
//
//     return BINI_ERR_SUCCESS;
// }

bini_section_t *bini_find_section(const bini_chunk_t *chunk, const char *name) {
    for (size_t i = 0; i < chunk->datafile.section_count; ++i) {
        bini_section_t *section = chunk->datafile.sections[i];
        if (strcmp(section->name, name) == 0)
            return section;
    }
    return NULL;
}

bini_section_t *bini_find_section_all(const bini_op_t *ops, const char *name, bini_chunk_t **containing_chunk) {
    for (size_t i = 0; i < ops->chunk_count; ++i) {
        bini_chunk_t *chunk = ops->chunks[i];
        if (!(chunk->type & CHUNK_DATAFILE_ANY))
            continue;

        bini_section_t *section = bini_find_section(chunk, name);
        if (section != NULL) {
            if (containing_chunk != NULL)
                *containing_chunk = chunk;
            return section;
        }
    }
    return NULL;
}

// TODO move into the multi module
bini_op_t *bini_parse_multi(tat_mempool_t *mempool, const bini_files_t *files) {
    const size_t files_count = files->count;

    if (files_count == 0)
        return NULL;

    const size_t owned_chunks_size = sizeof(bini_chunk_t) * files_count;
    const size_t pointer_array_size = sizeof(bini_chunk_t *) * files_count;
    const size_t buffer_size = sizeof(bini_op_t) + owned_chunks_size + pointer_array_size;

    char *buf = malloc(buffer_size);
    if (buf == NULL)
        return NULL;

    if (setjmp(bini_jump_buf)) {
        free(buf);
        return NULL;
    }

    bini_op_t *ops = (bini_op_t *) buf;
    ops->type = CHUNK_OP;
    ops->mempool = mempool;
    ops->chunk_count = 0;

    const union bini_files_u *files_array = files->files;

    // pointer array begins after owned chunks.
    ops->chunks = (bini_chunk_t **) buf + sizeof(bini_op_t) + owned_chunks_size;
    for (size_t i = 0; i < files_count; i++) {
        bini_chunk_t *chunk = &ops->owned_chunks[i];
        chunk->type = CHUNK_DATAFILE_INITIAL;
        ops->chunks[i] = chunk;
        ops->chunk_count += 1;

        bini_file_info_base_t *base = (bini_file_info_base_t *) (files_array + i);
        if (base->type == BINI_INVALID)
            continue;

        parse_file(ops, chunk, (bini_textfile_info_t *) base);
    }

    return ops;
}

void bini_free_ops(bini_op_t *ops) {
    free(ops);
}
