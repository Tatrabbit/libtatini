#ifndef TAT_TATINI_DEFINES_H
#define TAT_TATINI_DEFINES_H

#include "tat/libtatini.h"

#define VERSION_STRING "${APP_VERSION_NUMBER}"

// #if (${CMAKE_C_STANDARD} >= 23)
#define NODISCARD [[nodiscard]]
// #else
// #define NODISCARD
// #endif

#cmakedefine CMAKE_GCC_LIKE

#ifdef CMAKE_GCC_LIKE
#define WARN_UNUSED __attribute__((warn_unused_result))
#else
#define WARN_UNUSED
#endif

enum {
    ERR_SUCCESS = 0,

    ERR_USAGE = TATINI_ERR_COUNT,
};

#endif //DEFINES_IN_H
