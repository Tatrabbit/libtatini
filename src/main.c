#include "defines.h"

#include "tat_bini.h"
#include "tat_bini_infos.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    ERR_SUCCESS = 0,

    ERR_USAGE = BINI_ERR_COUNT,
};

typedef struct {
    const char *program_name;

    int verbose;
} options_t;

static options_t options;

#pragma region CLI Output

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
    return BINI_ERR_MEMORY;
}

static int print_file_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
    return BINI_ERR_FILE;
}

static void print_verbose(const char *fmt, ...) {
    if (!options.verbose)
        return;

    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
}

#pragma endregion

///@brief Find the basename of a file, in a platform-agnostic way.
static const char *platform_basename(const char *path) {
#   ifdef PLATFORM_WINDOWS
#       error "TODO"
#   endif

    const char *base;
    return ((base = strrchr(path, '/'))) ? (base + 1) : path;
}

typedef int (*filename_func_t_)(const char *filename, void *data);

#define filename_func_t WARN_UNUSED filename_func_t_

///@brief Call @ref func for all arguments.
///@param argc
///@param argv
///@param func Function to call. It should return 0 on success, or non-zero to abort.
///@param data Arbitrary data to be passed to @ref func
///@param argi The first argument to process. It's typically 0 or 1
WARN_UNUSED
static int iterate_argv_files_disambiguated(const unsigned int argc, const char *const *argv,
                                            const filename_func_t func, void *data,
                                            const unsigned int argi) {
    assert(argi < argc);

    for (size_t i = argi; i < argc; ++i) {
        int err;
        if ((err = func(argv[i], data)) != 0)
            return err;
    }
    return 0;
}

///@brief Call @ref func for all input filename arguments.
/// This function does not ensure the file exists.
///@param argc
///@param argv
///@param func Function to call. It should return 0 on success, or non-zero to abort.
///@param data Arbitrary data to be passed to @ref func
///@param argi The first argument to process. It's typically 0 or 1
WARN_UNUSED
static int iterate_argv_files(const unsigned int argc, const char *const *argv,
                              const filename_func_t func,
                              void *data, unsigned int argi) {
    assert(argi < argc);

    for (int i = argi; i < argc; ++i) {
        const char *arg = argv[i];

        // Skip all -o --options
        if (arg[0] == '-') {
            // Unless the option is -- in which case we will treat all as filenames.
            if (arg[1] == '-' && arg[2] == '\0')
                return iterate_argv_files_disambiguated(argc, &arg, func, data, i);
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

static int count_input_files(const unsigned int argc, const char *const *argv) {
    int err, count = 0;
    if ((err = iterate_argv_files(argc, argv, count_file_arg, &count, 1)))
        return -err;

    return count;
}

struct file_arg_data_s {
    bini_infos_t *infos;
    size_t loaded_count;
};

static int open_file_arg(const char *filename, void *data) {
    struct file_arg_data_s *arg_data = (struct file_arg_data_s *) data;


    const int err = bini_infos_open_one_t(arg_data->infos, arg_data->loaded_count, filename);
    if (err)
        return err; // TODO Use longjmp

    arg_data->loaded_count++;
    return 0;
}

static int open_input_files(const unsigned int argc, const char *const *argv, bini_infos_t *infos) {
    struct file_arg_data_s arg_data = {
        .infos = infos,
        .loaded_count = 0,
    };

    int err;
    if ((err = iterate_argv_files(argc, argv, open_file_arg, &arg_data, 1)))
        return err;

    return 0;
}

static int parse_options(const int argc, const char *const *argv) {
    options.program_name = platform_basename(argv[0]);
    if (argc < 2)
        return show_usage(1);
    return 0;
}

// static int cleanup(const int err) {
//     // cleanup_file_infos();
//     return err;
// }

int main(const int argc, char **argv) {
    int err = 0;
    bini_infos_t *infos = NULL;
    bini_op_t *ops = NULL;

    memset(&options, 0, sizeof(options));
    options.verbose = 1; // TODO

    if ((err = parse_options(argc, (const char **) argv)))
        goto cleanup;;

    show_version();

    const size_t input_count = count_input_files(argc, (const char **) argv);

    print_verbose("Reading info from %d files...\n", input_count);
    infos = bini_infos_allocate(input_count, NULL);
    if (!infos) {
        err = BINI_ERR_MEMORY;
        goto cleanup;
    }

    if ((err = open_input_files(argc, (const char **) argv, infos)))
        goto cleanup;

    bini_files_t files;

    if ((err = bini_infos_readall(infos, &files)))
        goto cleanup;

    ops = bini_parse_multi(files);

cleanup:
    if (ops != NULL)
        bini_free(ops);
    if (infos != NULL)
        bini_infos_free(infos);
    return err;
}
