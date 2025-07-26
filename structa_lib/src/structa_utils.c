#include "structa_utils.h"

structa_clamp(uint32_t val, uint32_t min, uint32_t max)
{
    return val < min ? min : (val > max ? max : val);
}

DWORD* structa_read_file(const char* file_path, size_t* file_size)
{
    HANDLE heap = GetProcessHeap();

    HANDLE file = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    DWORD size = GetFileSize(file, NULL);
    if (size == INVALID_FILE_SIZE)
    {
        CloseHandle(file);
        return NULL;
    }

    void* buffer = HeapAlloc(heap, HEAP_ZERO_MEMORY, size);
    if (buffer == NULL)
    {
        CloseHandle(file);
        return NULL;
    }

    DWORD bytesRead = 0;
    BOOL res = ReadFile(file, buffer, size, &bytesRead, NULL);
    CloseHandle(file);

    if (!res || bytesRead != size)
    {
        HeapFree(heap, 0, buffer);
        return NULL;
    }

    *file_size = size;

    return buffer;
}