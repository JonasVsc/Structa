#define CONSOLE_EXPORT 1
#include "console.h"

#if WINDOWS
    #include <windows.h>
#else
    #error
#endif

INTERNAL void console_set_error(console_error *error, bool occurred, uint code, void *buffer, uptr buffer_length, uptr buffer_size)
{
    error->occurred = occurred;
    error->code = code;
    error->buffer = buffer;
    error->buffer_length = buffer_length;
    error->buffer_size = buffer_size;
}

INTERNAL void console_get_error(console_error *error)
{
#if WINDOWS
    DWORD last_error = 0;
    DWORD length = 0;

    last_error = GetLastError();
    length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error->code, 0, error->buffer, error->buffer_size.low, 0);

    if(length > 0)
    {
        bool occurred = last_error != ERROR_SUCCESS;
        uptr buffer_length = {0};
        buffer_length.low = length;
        console_set_error(error, occurred, last_error, error->buffer, buffer_length, error->buffer_size);
    }
#else
    #error
#endif
}

CONSOLE_API void console_write_output(console_error *error, void *buffer, uptr buffer_length)
{
#if WINDOWS
    HANDLE output_console_handle = 0;

    console_get_error(error);
    output_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if(!output_console_handle || (output_console_handle == INVALID_HANDLE_VALUE))
    {
        console_get_error(error);
        return;
    }

    if(!WriteConsoleA(output_console_handle, buffer, buffer_length.low, 0, 0))
    {
        console_get_error(error);
        return;
    }
#else
    #error
#endif
}