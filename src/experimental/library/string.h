#ifndef STRING_H
#define STRING_H

#include "platform.h"

#ifdef STRING_EXPORT
    #define STRING_API EXPORT
#else
    #ifdef STRING_INCLUDE
        #define STRING_API INTERNAL
    #else
        #define STRING_API
    #endif
#endif

typedef platform_error string_error;

STRING_API uptr string_length(string_error *error, char *string);
STRING_API sint string_format(char *buffer, uptr buffer_size, char *format, ...);

#endif