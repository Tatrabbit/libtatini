#ifndef TAT_TATINI_CLI_ARGS_H
#define TAT_TATINI_CLI_ARGS_H

#include <stddef.h>

typedef int (*parse_func_t)(size_t *optc, const char *optv[], void *data);

typedef struct arg_handler_s {
    size_t n_names;
    const char **names;

    size_t n_options;

    parse_func_t func;
    void *data;
} arg_handler_t;

int parse_cli_arguments(unsigned int argc, const char **argv, arg_handler_t *handlers, size_t n_handlers);

#endif
