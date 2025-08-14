#ifndef STRUCTA_LOADER_H_
#define STRUCTA_LOADER_H_ 1

#include <stdbool.h>
#include <time.h>

typedef struct StructaModule_T StructaModule_T;
typedef struct StructaModule_T* StructaModule;

void StructaLoadModule(StructaModule module, const char* path);

void StructaFreeModule(StructaModule module);

void* StructaLoaderGetFunc(StructaModule module, const char* funcName);

bool StructaCheckVersion(StructaModule module);

time_t GetFileTimeStamp(const char* path);

#endif // STRUCTA_LOADER_H_