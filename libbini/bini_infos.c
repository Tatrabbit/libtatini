#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tat_bini.h"
#include "tat_bini_infos.h"

#include "bini_types.h"

static size_t get_fsize(FILE *f) {
    fseek(f, 0, SEEK_END);
    const size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

bini_infos_t *bini_infos_allocate(size_t count, bini_infos_t *infos) {
    const size_t size_each = sizeof(union bini_files_u);
    const size_t total_size = sizeof(bini_infos_t) + size_each * count;
    size_t clear_start = 0;

    if (infos != NULL) {
        assert(infos->buffer == NULL);

        clear_start = sizeof(bini_infos_t) + infos->count * size_each;
        count += infos->count;
    }

    infos = realloc(infos, total_size);
    if (infos == NULL)
        return NULL;

    memset(infos, clear_start, total_size - clear_start);
    infos->count = count;

    return infos;
}

static void close_info(struct bini_file_info_base_s *info) {
    if (info->handle != NULL) {
        fclose(info->handle);
        info->handle = NULL;
    }
}

void bini_infos_free(bini_infos_t *infos) {
    assert(infos != NULL);

    for (size_t i = 0; i < infos->count; i++) {
        const struct bini_file_info_base_s *info = &infos->files[i].base;

        if (info->handle != NULL)
            fclose(info->handle);
    }

    if (infos->buffer != NULL)
        free(infos->buffer);

    free(infos);
}

int bini_infos_open_one_t(bini_infos_t *infos, const size_t index, const char *filename) {
    assert(index < infos->count);
    assert(filename != NULL);

    FILE *handle = fopen(filename, "rb");
    if (handle == NULL)
        return 1;

    bini_textfile_info_t *info = &infos->files[index].text;
    close_info((struct bini_file_info_base_s *) info);
    memset(info, 0, sizeof(*info));

    info->type = BINI_TEXTFILE;
    info->handle = handle;
    info->filename = filename;

    info->file_size = get_fsize(handle);
    infos->initial_buffer_size += info->file_size;

    return 0;
}

static int bini_read_textfile_info(bini_textfile_info_t *info, char **shared_buffer) {
    if (info->handle == NULL)
        return BINI_ERR_STATE;

    char *buffer = *shared_buffer;

    fseek(info->handle, 0, SEEK_SET);
    const size_t read_count = fread(buffer, info->file_size - 1, 1, info->handle);
    if (read_count != 1)
        return BINI_ERR_FILE;

    info->contents = buffer;
    *shared_buffer += info->file_size;

    return 0;
}

#define GET_BASE_INFO(x, i)((struct bini_file_info_base_s *) (x->files + i))
#define GET_TEXT_INFO(x, i)((bini_textfile_info_t *) (x->files + i))

int bini_infos_readall(bini_infos_t *infos, bini_files_t *bini_files) {
    assert(infos != NULL);
    assert(infos->buffer == NULL);

    if (infos->count == 0)
        return 0;

    // Allocate all buffers
    if (infos->initial_buffer_size == 0) {
        return 0;
    }

    infos->buffer = malloc(infos->initial_buffer_size);
    if (infos->buffer == NULL)
        return BINI_ERR_MEMORY;

    // Read & assign buffers
    char *shared_buffer = infos->buffer;
    for (size_t i = 0; i < infos->count; ++i) {
        const int err = bini_read_textfile_info(GET_TEXT_INFO(infos, i), &shared_buffer);
        if (err) {
            free(infos->buffer);
            infos->buffer = NULL;
            return err;
        }
    }

    for (size_t i = 0; i < infos->count; ++i)
        close_info(GET_BASE_INFO(infos, i));

    bini_files->count = infos->count;
    bini_files->files = infos->files;
    return 0;
}
