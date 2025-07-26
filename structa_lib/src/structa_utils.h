#ifndef STRUCTA_UTILS_H_
#define STRUCTA_UTILS_H_ 1

#include <Windows.h>
#include <stdint.h>

structa_clamp(uint32_t val, uint32_t min, uint32_t max);

DWORD* structa_read_file(const char* file_path, size_t* file_size);

#endif // STRUCTA_UTILS_H_