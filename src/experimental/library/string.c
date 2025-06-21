#define STRING_EXPORT 1
#include "string.h"

typedef uptr *va_args;

#define STRING_VA_GET(argument_list, type) *(type *)string_va_get_argument(argument_list)

INTERNAL va_args string_va_initialize(void *format)
{
    va_args result = (va_args)format + 1;
    return result;
}

INTERNAL va_args string_va_finalize()
{
    va_args result = 0;
    return result;
}

INTERNAL void *string_va_get_argument(va_args *argument_list)
{
    void *result = (*argument_list)++;
    return result;
}

INTERNAL void string_set_error(string_error *error, bool occurred, uint code, void *buffer, uptr buffer_length, uptr buffer_size)
{
    error->occurred = occurred;
    error->code = code;
    error->buffer = buffer;
    error->buffer_length = buffer_length;
    error->buffer_size = buffer_size;
}

STRING_API uptr string_length(string_error *error, char *string)
{
    uptr result = {0};
    string_error error2;

    while(*string != '\0')
    {
        if(result.low == MAX_U32)
        {
            if(result.high == MAX_U32)
            {
                char *string_error = "String is too long.";
                string_set_error(error, 1, 0, string_error, string_length(&error2, string_error), error->buffer_size);
                return result;
            }
            else
            {
                result.high++;
            }
        }
        else
        {
            result.low++;
        }
    }

    return result;
}

STRING_API sint string_format(char *buffer, uptr buffer_size, char *format, ...)
{
    sint result = 0;
    va_args argument_list = 0;
    char *c = format;

    argument_list = string_va_initialize(&format);
    
    while(*c != '\0')
    {

    }

    argument_list = string_va_finalize();

    return result;
}