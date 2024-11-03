#define TAT_LIBTATINI_C
#include "./include/tat/libtatini.h"

// TODO should be optional!
#include "./include/tat/libtatini_mempool.h"

#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#define MEMCHUNK_SIZE 4096

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

static tatini_section_ref_t *new_section(tatini_mempool_t *mempool, const char *name) {
    tatini_section_ref_t *section = tatini_mempool_getmem(mempool, sizeof(tatini_section_ref_t));

    section->name = name;
    section->key_count = 0;
    section->keys = NULL;
    section->last_key = NULL;

    return section;
}

void tatini_parse_inplace(tatini_mempool_t *mempool, tatini_file_t *file) {
    char *next = file->contents;
    const char *end = file->contents + file->size;

    tatini_section_ref_t *current_section = new_section(mempool, NULL);

    char *line;
    while ((line = split_mem_line(&next, end))) {
        printf("Parsing: %s: \"%s\"", file->name, line);
        const char *section_name = parse_line_section1(line);

        if (section_name) {
            current_section = new_section(mempool, section_name);
            printf(" = Section: \"%s\"\n", current_section->name);
        } else
            printf(" (Nothing found.)\n");
    }
}

static tatini_section_ref_t *find_first_section_in(const tatini_chunk_t *chunk, const char *name) {

    for (size_t i = 0; i < chunk->n_sections; ++i) {
        tatini_section_ref_t *section = &chunk->sections[i];
        if (strcmp(section->name, name) == 0)
            return section;
    }
    return NULL;
}

tatini_section_ref_t *tatini_section_find_first(const tatini_state_t *state, const char *name) {
    for (size_t i = 0; i < state->n_chunks; ++i) {
        const tatini_chunk_t *chunk = state->chunks[i];

        tatini_section_ref_t *section = find_first_section_in(chunk, name);
        if (section != NULL)
            return section;
    }
    return NULL;
}

void tatini_state_free(tatini_state_t *state) {
    free(state);
}
