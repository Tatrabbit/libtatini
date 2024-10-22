#ifndef TAT_TATINI_TYPES_H
#define TAT_TATINI_TYPES_H

#include <stdio.h>

enum {
    TATINI_INVALID = 0,
    TATINI_TEXTFILE = 1,
};

// TODO no need for this any more, remove all, use just text

#define BINI_FLAGS_PADDING_(s) char _[sizeof(const char *) - s];

typedef struct bini_file_info_base_s {
    char type;
    BINI_FLAGS_PADDING_(1)

    ///@brief File name
    const char *filename;

    ///@brief File handle
    FILE *handle;
} bini_file_info_base_t;

typedef struct tatini_textfile_info_s {
    char type;
    BINI_FLAGS_PADDING_(1)

    const char *filename;
    FILE *handle;

    ///@brief Known size of file
    size_t file_size;

    ///@brief Existing buffer containing file contents.
    ///@note This will be modified during parsing
    char *contents;
} bini_textfile_info_t;

union tatini_files_u {
    int type;
    bini_file_info_base_t base;
    bini_textfile_info_t text;
};

// TODO binfile

#undef BINI_FLAGS_PADDING_

#endif
