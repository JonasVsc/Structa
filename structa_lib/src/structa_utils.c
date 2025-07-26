#include "structa_utils.h"

uint32_t structa_clamp(uint32_t val, uint32_t min, uint32_t max)
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

uint32_t structa_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties = { 0 };
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if ((type_filter & (i << 1)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return -1;
}