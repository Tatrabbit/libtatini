#include "defines.h"

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
    const char *filename;
    size_t size;
    char *contents;
} file_info_t;

typedef struct {
    const char *program_name;
    file_info_t *file_infos;

    int verbose;
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

static int parse_file(const char *file_name, char **buffer) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
        return 1;
    const size_t size = get_fsize(file);

    *buffer = malloc(size + 1);
    if (*buffer == NULL)
        return 1;

    fseek(file, 0, SEEK_SET);
    fread(buffer, size, 1, file);
    fclose(file);

    return 0;
}

typedef void (*filename_func_t)(const char *filename, void *data);

static int iterate_argv_files_disambiguated(const int argc, const char *const *argv, const filename_func_t func,
                                            void *data) {
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "--") == 0)
            return print_usage_error("only one -- permitted\n");

        func(arg, data);
    }
    return 0;
}

static int iterate_argv_files(const int argc, const char *const *argv, const filename_func_t func, void *data) {
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (arg[0] == '-') {
            if (arg[1] == '-' && arg[2] == '\0')
                return iterate_argv_files_disambiguated(argc, &arg, func, data);
            continue;
        }
        func(arg, data);
    }

    return 0;
}

static void count_file_arg(const char *filename, void *data) {
    (void) filename;
    *(int *) data += 1;
}

static void get_file_arg(const char *filename, void *data) {
    size_t *index = data;
    options.file_infos[*index].filename = filename;
    *index += 1;
}

static int count_input_files(int argc, const char *const *argv) {
    int count = 0;
    iterate_argv_files(argc, argv, count_file_arg, &count);
    return count;
}

static int get_input_filenames(int argc, const char *const *argv) {
    const int file_count = count_input_files(argc, argv);

    if (file_count <= 0) {
        print_info("No input files.");
        return 0;
    }
    print_verbose("Reading info from %d files...\n", file_count);

    options.file_infos = malloc(sizeof(file_info_t));

    if (options.file_infos == NULL) {
        return print_mem_error();
    }

    size_t file_arg_index = 0;
    (void) iterate_argv_files(argc, argv, get_file_arg, &file_arg_index);

    for (size_t i = 0; i < file_count; ++i) {
        print_info("Opening file %s\n", options.file_infos[i].filename);
        // TODO
    }

    return 0;
}

static int parse_input_files() {
    return 0;
}

static int parse_options(const int argc, const char *const *argv) {
    options.program_name = platform_basename(argv[0]);
    if (argc < 2)
        return show_usage(1);
    return 0;
}

int main(const int argc, char **argv) {
    memset(&options, 0, sizeof(options));
    options.verbose = 1; // TODO

    show_version();

    int err;

    if ((err = parse_options(argc, (const char **) argv)))
        goto cleanup;

    if ((err = get_input_filenames(argc, (const char **) argv)))
        goto cleanup;

    err = parse_input_files();

cleanup:
    if (options.file_infos != NULL)
        free(options.file_infos);

    return err;
}
