#include "defines.h"

#include "tat/libtatini.h"
#include "tat/libtatini_infos.h"

#include "tat/libtatini_mempool.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_args.h"

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
    print_info("Tini: Teeny INI tool by Tatrabbit (Version " VERSION_STRING ")\n");
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
    return TATINI_ERR_MEMORY;
}

static int print_file_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprint_info(fmt, ap);
    va_end(ap);
    return TATINI_ERR_FILE;
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

#define filename_func_t WARN_UNUSED filename_func_t_

static int arg_count_file(size_t *optc, const char *optv[], void *data) {
    assert(*optc == 1); // == n_options
    size_t *p_count = data;

    *p_count += 1; // Blindly count it, check later.
    return ERR_SUCCESS;
}

static int cli_count_input_files(const unsigned int argc, const char **argv, arg_handler_t *handler_data, size_t *count) {
    handler_data->func = arg_count_file;
    handler_data->data = count;

    *count = 0;
    return parse_cli_arguments(argc, argv, handler_data, 1);
}

struct file_arg_data_s {
    tatini_infos_t *infos;
    size_t loaded_count;
};


static int arg_open_file(size_t *optc, const char *optv[], void *data) {
    struct file_arg_data_s *arg_data = (struct file_arg_data_s *) data;

    assert(*optc == 1); // == n_options
    const char *filename = optv[0];

    int err;
    if ((err = tatini_infos_open_one_t(arg_data->infos, arg_data->loaded_count, filename)))
        return err; // TODO Use longjmp

    arg_data->loaded_count++;
    return ERR_SUCCESS;
}

static int cli_open_input_files(const unsigned int argc, const char **argv, arg_handler_t *handler_data, tatini_infos_t *infos) {
    struct file_arg_data_s arg_data = {
        .infos = infos,
        .loaded_count = 0,
    };

    handler_data->func = arg_open_file;
    handler_data->data = &arg_data;

    return parse_cli_arguments(argc, argv, handler_data, 1);
}

static int open_input_files(const unsigned int argc, const char **argv, tatini_infos_t **allocated_infos) {
    int err;

    arg_handler_t handler_data = (arg_handler_t){
        .n_names = 2,
        .names = (const char *[2]){"-i", "--input"},

        .n_options = 1,

        // .func = ??
        // .data = ??
    };

    size_t input_count;
    if ((err = cli_count_input_files(argc, argv, &handler_data, &input_count)))
        return err;

    print_verbose("Reading info from %d files...\n", input_count);

    *allocated_infos = tatini_infos_allocate(input_count, NULL);
    if (!*allocated_infos)
        return TATINI_ERR_MEMORY;

    return cli_open_input_files(argc, argv, &handler_data, *allocated_infos);
}

int main(const int argc, char **argv) {
    int err = ERR_SUCCESS;
    tatini_mempool_t *mempool = NULL;
    tatini_infos_t *infos = NULL;
    tatini_state_t *state = NULL;

    memset(&options, 0, sizeof(options));

    options.program_name = platform_basename(argv[0]);
    options.verbose = 1; // TODO

    show_version();

    if ((err = open_input_files(argc, (const char **)argv, &infos)))
        goto cleanup;

    if ((err = tatini_infos_read_all(infos)))
        goto cleanup;

    mempool = tatini_mempool_new(4096);
    if (!mempool) {
        err = TATINI_ERR_MEMORY;
        goto cleanup;
    }

    state = tatini_infos_parse(mempool, infos);

cleanup:
    if (state != NULL)
        tatini_state_free(state);

    if (mempool != NULL)
        tatini_mempool_free(mempool);

    if (infos != NULL)
        tatini_infos_free(infos);

    return err;
}
