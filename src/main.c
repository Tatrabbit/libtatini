#include "defines.h"

#include "tat_bini.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    ERR_SUCCESS = 0,

    ERR_USAGE = -1,

    ERR_MEMORY = -4,
    ERR_FILE = -5,
};

typedef struct {
    bini_textfile_info_t bini;
    FILE *handle;
} file_info_t;

typedef struct {
    const char *program_name;

    size_t input_count;

    int verbose;

    union {
        void *pointer;
        file_info_t *inputs;
        bini_textfile_info_t *bini;
    } file_info;
} options_t;

static options_t options;

static void vprint_info(const char *fmt, const va_list ap) {
    vfprintf(stderr, fmt, ap);
}

static void print_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
}

static void show_version() {
    print_info("Bini-Knife by Tatrabbit (Version " VERSION_STRING ")\n");
}

static int show_usage(const int code) {
    printf("Usage: %s input.ini [input2.ini...]\n", options.program_name);
    return code;
}

static int print_usage_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
    return ERR_USAGE;
}

static int print_mem_error() {
    fputs("Out of memory.\n", stderr);
    return ERR_MEMORY;
}

static int print_file_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
    return ERR_FILE;
}

static void print_verbose(const char *fmt, ...) {
    if (!options.verbose)
        return;

    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
}

static const char *platform_basename(const char *path) {
#   ifdef PLATFORM_WINDOWS
#       error "TODO"
#   endif

    const char *base;
    return ((base = strrchr(path, '/'))) ? (base + 1) : path;
}

static size_t get_fsize(FILE *f) {
    fseek(f, 0, SEEK_END);
    const size_t size = ftell(f);
    return size;
}

typedef int (*filename_func_t_)(const char *filename, void *data);

#define filename_func_t WARN_UNUSED filename_func_t_

WARN_UNUSED
static int iterate_argv_files_disambiguated(const int argc, const char *const *argv,
                                            const filename_func_t func, void *data) {
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "--") == 0)
            return print_usage_error("only one -- permitted\n");

        int err;
        if ((err = func(arg, data)) != 0)
            return err;
    }
    return 0;
}

WARN_UNUSED
static int iterate_argv_files(const int argc, const char *const *argv, const filename_func_t func, void *data) {
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (arg[0] == '-') {
            if (arg[1] == '-' && arg[2] == '\0')
                return iterate_argv_files_disambiguated(argc, &arg, func, data);
            continue;
        }

        int err;
        if ((err = func(arg, data)) != 0)
            return err;
    }

    return 0;
}

static int count_file_arg(const char *filename, void *data) {
    (void) filename;
    *(int *) data += 1;
    return 0;
}

static int count_input_files(const int argc, const char *const *argv) {
    int err, count = 0;
    if ((err = iterate_argv_files(argc, argv, count_file_arg, &count)))
        return -err;

    return count;
}

typedef struct {
    size_t index;
    size_t total_size;
} file_arg_data_t;

static int get_file_arg(const char *filename, void *data) {
    file_arg_data_t *arg_data = data;
    file_info_t *info = &options.file_info.inputs[arg_data->index++];

    print_verbose("Opening file #%d (%s)\n", arg_data->index, filename);

    info->handle = fopen(filename, "r");
    if (info->handle == NULL)
        return print_file_error("Cannot open %s", filename);

    const size_t size = get_fsize(info->handle) + 1;
    info->bini.file_size = size;
    arg_data->total_size += size;
    return 0;
}

static int allocate_input_files(const int argc, const char *const *argv) {
    options.input_count = count_input_files(argc, argv);
    if (options.input_count == 0)
        return 0;

    if (options.input_count < 0) // ERROR
        return options.input_count;

    print_verbose("Reading info from %d files...\n", options.input_count);

    // Allocate and zerofill
    options.file_info.pointer = malloc(sizeof(file_info_t) * options.input_count);
    if (options.file_info.pointer == NULL) {
        return print_mem_error();
    }
    memset(options.file_info.pointer, 0, sizeof(file_info_t) * options.input_count);

    return 0;
}

static int read_input_files(const int argc, const char *const *argv) {
    int err;

    // Get filename, count file arguments
    if ((err = allocate_input_files(argc, argv)) != 0)
        return err;

    if (options.input_count == 0) {
        print_info("No input files.");
        return 0;
    }

    // Open, each get size
    file_arg_data_t arg_data = {.index = 0, .total_size = 0};
    if ((err = iterate_argv_files(argc, argv, get_file_arg, &arg_data)))
        return err;

    // Allocate all buffers
    char *buffer = malloc(arg_data.total_size);
    if (buffer == NULL)
        return print_mem_error();

    // Read & assign buffers
    for (size_t i = 0; i < options.input_count; ++i) {
        file_info_t *info = &options.file_info.inputs[i];

        fseek(info->handle, 0, SEEK_SET);
        const size_t read_count = fread(buffer, info->bini.file_size - 1, 1, info->handle);
        if (read_count != 1)
            return print_file_error("Error reading file #%d (%s)\n", i, info->bini.filename);

        info->bini.contents = buffer;
        buffer += info->bini.file_size;
    }

    return 0;
}

static int parse_options(const int argc, const char *const *argv) {
    options.program_name = platform_basename(argv[0]);
    if (argc < 2)
        return show_usage(1);
    return 0;
}

static void close_input_files() {
    if (options.input_count == 0)
        return;

    for (size_t i = 0; i < options.input_count; ++i) {
        file_info_t *info = &options.file_info.inputs[i];

        FILE *file = info->handle;
        if (file == NULL)
            break;

        fclose(file);
        info->handle = NULL;
    }

    options.input_count = 0;
}

static void cleanup_file_infos() {
    if (options.file_info.pointer == NULL)
        return;

    close_input_files();

    if (options.file_info.bini[0].contents != NULL)
        free(options.file_info.bini[0].contents);

    free(options.file_info.pointer);
    options.file_info.pointer = NULL;
}

static int cleanup(const int err) {
    cleanup_file_infos();
    return err;
}

static size_t convert_fileinfo_array() {
    assert(sizeof(bini_textfile_info_t) < sizeof(file_info_t));

    const size_t count = options.input_count;
    close_input_files();

    for (size_t i = 0; i < count; ++i)
        options.file_info.bini[i] = *(bini_textfile_info_t *) &options.file_info.inputs[i];

    return count;
}

int main(const int argc, char **argv) {
    memset(&options, 0, sizeof(options));
    options.verbose = 1; // TODO

    show_version();

    int err;
    if ((err = parse_options(argc, (const char **) argv)))
        return cleanup(err);

    if ((err = read_input_files(argc, (const char **) argv)))
        return cleanup(err);

    const size_t count = convert_fileinfo_array();
    bini_parseini_inplace_multi(options.file_info.bini, count);

    return cleanup(0);
}
