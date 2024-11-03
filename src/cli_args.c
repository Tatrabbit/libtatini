#include "cli_args.h"

#include <assert.h>
#include <string.h>

#include "defines.h"

static arg_handler_t *find_handler(const char *name, arg_handler_t *handlers, size_t n_handlers) {
    // For each handler
    for (size_t i = 0; i < n_handlers; ++i) {
        arg_handler_t *handler = handlers + i;

        // Try each name
        for (size_t name_i = 0; name_i < handler->n_names; ++name_i) {
            const char *handler_name = handler->names[name_i];
            if (strcmp(handler_name, name) == 0)
                return handler;
        }
    }

    return NULL;
}

int parse_cli_arguments(const unsigned int argc, const char *argv[], arg_handler_t *handlers, const size_t n_handlers) {
    // For each CLI argument
    for (size_t argi = 1; argi < argc; ++argi) {
        // Find matching handler
        const arg_handler_t *handler = find_handler(argv[argi], handlers, n_handlers);
        if (handler == NULL)
            return ERR_USAGE;

        // Call handler
        int err;
        size_t n_options = handler->n_options;
        const char **optv = argv + argi + n_options;
        if ((err=handler->func(&n_options, optv, handler->data)))
            return err;

        // Skip handled options
        assert(n_options <= handler->n_options);
        argi += n_options;
    }

    return ERR_SUCCESS;
}

