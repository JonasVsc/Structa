#ifndef STRUCTA_UTILS_H_
#define STRUCTA_UTILS_H_ 1

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

inline uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) { return val < min ? min : (val > max ? max : val); }

DWORD* StructaReadFile(const char* file_path, size_t* file_size);

#endif // STRUCTA_UTILS_H_