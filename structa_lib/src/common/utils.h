#ifndef STRUCTA_UTILS_H_
#define STRUCTA_UTILS_H_ 1

#include <Windows.h>
#include <vulkan/vulkan.h>
#include <stdint.h>

uint32_t clamp(uint32_t val, uint32_t min, uint32_t max);

DWORD* read_file(const char* file_path, size_t* file_size);

uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

#endif // STRUCTA_UTILS_H_