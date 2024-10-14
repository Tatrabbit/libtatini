#include "bini.h"

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

char *bini__memchunk_get(const size_t size);

void bini__memchunk_free();

jmp_buf bini_jump_buf;

enum {
    CHUNK_NONE = 0,

    CHUNK_DATAFILE_INITIAL = 0x1,
    CHUNK_DATAFILE_OWNED = 0x2,
    CHUNK_DATAFILE_ANY = 0x3,
};

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

static bini_section_t *new_section(const char *name) {
    bini_section_t *section = (bini_section_t *) bini__memchunk_get(sizeof(bini_section_t));

    section->name = name;
    section->key_count = 0;
    section->keys = NULL;
    section->last_key = NULL;

    return section;
}

static void parse_file(const bini_op_t *ops, bini_chunk_t *chunk, const ini_file_t *file) {
    char *next = file->contents;
    const char *end = file->contents + file->file_size;

    bini_section_t *current_section = new_section(NULL);

    char *line;
    while ((line = split_mem_line(&next, end))) {
        printf("Parsing: \"%s\"", line);
        const char *section_name = parse_line_section1(line);

        if (section_name) {
            // current_section = bini_find_section_all(ops, section_name, NULL);
            current_section = new_section(section_name);
            printf(" = Section: \"%s\"\n", current_section->name);
        } else
            printf(" (Nothing found.)\n");
    }
}

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


bini_op_t *bini_parseini_inplace_multi(const ini_file_t *file, const size_t file_count) {
    if (file_count == 0)
        return NULL;

    const size_t owned_chunks_size = sizeof(bini_chunk_t) * file_count;
    const size_t pointer_array_size = sizeof(bini_chunk_t *) * file_count;
    const size_t buffer_size = sizeof(bini_op_t) + owned_chunks_size + pointer_array_size;

    char *buf = malloc(buffer_size);
    if (buf == NULL)
        return NULL;

    if (setjmp(bini_jump_buf)) {
        bini__memchunk_free();
        free(buf);
        return NULL;
    }

    bini_op_t *ops = (bini_op_t *) buf;
    // ops->chunk_count = file_count;
    ops->chunk_count = 0;

    // pointer array begins after owned chunks.
    ops->chunks = (bini_chunk_t **) buf + sizeof(bini_op_t) + owned_chunks_size;
    for (size_t i = 0; i < file_count; i++) {
        bini_chunk_t *chunk = &ops->owned_chunks[i];
        chunk->type = CHUNK_DATAFILE_INITIAL;
        ops->chunks[i] = chunk;
        ops->chunk_count += 1;

        parse_file(ops, chunk, file++);
    }

    return ops;
}
