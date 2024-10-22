///@brief Allocate space efficiently for opening and reading multiple files at once
///
///@note This module is meant for when the number of file to be opened is known, and they are to
///      be opened and parsed as one, atomic operation.
#ifndef TAT_BINI_INFOS_H
#define TAT_BINI_INFOS_H

// TODO restructure this whole directory...
#include "../../bini_types.h"

#include <stddef.h>

// TODO opaque pointers

typedef struct {
    size_t count;
    size_t initial_buffer_size;

    char *buffer;

    union bini_files_u files[];
} bini_infos_t;

///@brief Allocate an Infos batch
///@param count The number of files to allocate
///@param append If not NULL, realloc will be used to extend the size of an existing batch.
///@return The newly (re)allocated infos batch, or NULL on memory error
///@note The returned pointer must be passed to @see bini_infos_free to avoid memory leaks.
bini_infos_t *bini_infos_allocate(size_t count, bini_infos_t *append);

void bini_infos_free(bini_infos_t *infos);

///@brief Atomically open one text file in the batch.
///
/// On success, the file's size is checked, the @ref filename provided is stored, and the handle remains open.
/// On I/O failure, the state of both @ref infos and @ref index is unchanged.
///@param infos The batch to operate on
///@param index The file index to open.
///@param filename The file name to set.
///@return On success, 0 is returned. Otherwise, the error code.
int bini_infos_open_one_t(bini_infos_t *infos, size_t index, const char *filename);

int bini_infos_readall(bini_infos_t *infos, bini_files_t *bini_files);

#endif
