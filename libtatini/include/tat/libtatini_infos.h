///@brief Allocate space efficiently for opening and reading multiple files at once
///
///@note This module is meant for when the number of file to be opened is known, and they are to
///      be opened and parsed as one, atomic operation.
#ifndef TAT_LIBTATINI_INFOS_H
#define TAT_LIBTATINI_INFOS_H

#include "./libtatini.h"

typedef struct tatini_infos_s {
    size_t count;
    size_t initial_buffer_size;

    char *buffer;

    tatini_file_t files[];
} tatini_infos_t;

///@brief Allocate an Infos batch
///@param count The number of files to allocate
///@param append If not NULL, realloc will be used to extend the size of an existing batch.
///@return The newly (re)allocated infos batch, or NULL on memory error
///@note The returned pointer must be passed to @see bini_infos_free to avoid memory leaks.
tatini_infos_t *tatini_infos_allocate(size_t count, tatini_infos_t *append);

void tatini_infos_free(tatini_infos_t *infos);

///@brief Atomically open one text file in the batch.
///
/// On success, the file's size is checked, the @ref filename provided is stored, and the handle remains open.
/// On I/O failure, the state of both @ref infos and @ref index is unchanged.
///@param infos The batch to operate on
///@param index The file index to open.
///@param filename The file name to set.
///@return On success, 0 is returned. Otherwise, the error code.
int tatini_infos_open_one_t(tatini_infos_t *infos, size_t index, const char *filename);


// TODO combine with parse
int tatini_infos_read_all(tatini_infos_t *infos);

// * Each state holds allocated strings for when key/value pairs are added or modified.
// * Depending on your use case, you may choose to create one state for all files, or associate
// * a state with a specific file or set of files. Use one global state if your use case is "one and done"
// * AKA you need to parse a file or a set of them, and then no more usage is needed.

// * \note Added parameters, created strings, etc. are only valid while this object exists.
// *       To prevent memory leaks, \ref bini_free() must be called.
tatini_state_t *tatini_infos_parse(tatini_mempool_t *mempool, tatini_infos_t *infos);

#endif
