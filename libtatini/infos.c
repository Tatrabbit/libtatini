#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "./include/tat/libtatini_infos.h"

static size_t get_fsize(FILE *f) {
    fseek(f, 0, SEEK_END);
    const size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

tatini_infos_t *tatini_infos_allocate(size_t count, tatini_infos_t *infos) {
    const size_t size_each = sizeof(tatini_file_t);
    const size_t total_size = sizeof(tatini_infos_t) + size_each * count;
    size_t clear_start = 0;

    if (infos) {
        assert(infos->buffer == NULL);

        clear_start = sizeof(tatini_infos_t) + infos->count * size_each;
        count += infos->count;
    }

    infos = realloc(infos, total_size);
    if (!infos)
        return NULL; // TODO error handling

    memset(infos, 0, total_size - clear_start);
    infos->count = count;

    return infos;
}

static void close_file(tatini_file_t *file) {
    if (file->handle) {
        fclose(file->handle);
        file->handle = NULL;
    }
}

void tatini_infos_free(tatini_infos_t *infos) {
    assert(infos != NULL);

    for (size_t i = 0; i < infos->count; i++) {
        const tatini_file_t *info = &infos->files[i];

        if (info->handle)
            fclose(info->handle);
    }

    if (infos->buffer)
        free(infos->buffer);

    free(infos);
}

int tatini_infos_open_one_t(tatini_infos_t *infos, const size_t index, const char *filename) {
    assert(index < infos->count);
    assert(filename != NULL);

    FILE *handle = fopen(filename, "rb");
    if (!handle)
        return 1; // TODO error handling

    tatini_file_t *info = &infos->files[index];
    close_file(info);
    memset(info, 0, sizeof(*info));

    info->name = filename;
    info->handle = handle;

    info->size = get_fsize(handle);
    infos->initial_buffer_size += info->size;

    return 0;
}

static int bini_read_textfile_info(tatini_file_t *info, char **shared_buffer) {
    if (!info->handle)
        return TATINI_ERR_STATE;

    char *buffer = *shared_buffer;

    fseek(info->handle, 0, SEEK_SET);
    const size_t read_count = fread(buffer, info->size - 1, 1, info->handle);
    if (read_count != 1)
        return TATINI_ERR_FILE;

    info->contents = buffer;
    *shared_buffer += info->size;

    return 0;
}

int tatini_infos_read_all(tatini_infos_t *infos) {
    assert(infos != NULL);
    assert(infos->buffer == NULL);

    if (infos->count == 0)
        return 0;

    // Allocate all buffers
    if (infos->initial_buffer_size == 0) {
        return 0;
    }

    infos->buffer = malloc(infos->initial_buffer_size);
    if (!infos->buffer)
        return TATINI_ERR_MEMORY; // TODO error handling

    // Read & assign buffers
    char *shared_buffer = infos->buffer;
    for (size_t i = 0; i < infos->count; ++i) {
        const int err = bini_read_textfile_info(&infos->files[i], &shared_buffer);
        if (err) {
            free(infos->buffer);
            infos->buffer = NULL;
            return err;
        }
    }

    for (size_t i = 0; i < infos->count; ++i)
        close_file(&infos->files[i]);

    return 0;
}

tatini_state_t *tatini_infos_parse(tatini_mempool_t *mempool, tatini_infos_t *infos) {
    const size_t files_count = infos->count;

    if (files_count == 0)
        return NULL;

    const size_t owned_chunks_size = sizeof(tatini_chunk_t) * files_count;
    const size_t pointer_array_size = sizeof(tatini_chunk_t *) * files_count;
    const size_t buffer_size = sizeof(tatini_state_t) + owned_chunks_size + pointer_array_size;

    char *buf = malloc(buffer_size);
    if (buf == NULL)
        return NULL;

    // if (setjmp(tatini_jump_buf)) {
    //     free(buf);
    //     return NULL;
    // }

    tatini_state_t *state = (tatini_state_t *) buf;

    state->n_chunks = 0;
    state->chunks = (tatini_chunk_t **)(buf + sizeof(*state));

    for (size_t i = 0, j = 0; i < files_count; i++) {
        tatini_file_t *file = &infos->files[i];

        tatini_parse_inplace(mempool, file); // TODO error handling
        state->chunks[state->n_chunks++] = (tatini_chunk_t *)file; // upcast
    }

    return state;
}