#ifndef CONSOLE_H
#define CONSOLE_H

#include "platform.h"

#ifdef CONSOLE_EXPORT
    #define CONSOLE_API EXPORT
#else
    #ifdef CONSOLE_INCLUDE
        #define CONSOLE_API INTERNAL
    #else
        #define CONSOLE_API
    #endif
#endif

typedef platform_error console_error;

CONSOLE_API void console_write_output(console_error *error, void *buffer, uptr buffer_size);

#endif