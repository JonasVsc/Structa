#include "library/console.h"

int main(int argument_count, char **argument_list)
{
    char buffer[512];

    platform_error error = {0};
    error.buffer = buffer;
    error.buffer_size.low = sizeof(buffer);

    char *string = "Hello\n";
    uptr string_size = {0};
    string_size.low = 6;

    console_write_output(&error, string, string_size);

    return 0;
}