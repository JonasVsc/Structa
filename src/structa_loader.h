#ifndef STRUCTA_LOADER_H_
#define STRUCTA_LOADER_H_ 1

#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct StructaModule_T {
	const char* path;
	void* handle;
	time_t lastWrite;
} StructaModule_T;
typedef struct StructaModule_T* StructaModule;

// path is optional if module was loaded one time
void StructaLoadModule(StructaModule module, const char* path);
void StructaFreeModule(StructaModule module);
void* StructaLoaderGetFunc(StructaModule module, const char* funcName);
// return true when has a new update
bool StructaCheckVersion(StructaModule module);
time_t GetFileTimeStamp(const char* path);

inline void StructaLoadModule(StructaModule module, const char* path)
{
	if (path != NULL)
	{
		module->path = path;
	}

	module->lastWrite = GetFileTimeStamp(module->path);
	
	char dll[256];
	snprintf(dll, sizeof(dll), "_%s_%lld.dll", module->path, module->lastWrite);

	printf("Copiando  %s para %s\n", module->path, dll);
	while (!CopyFileA(module->path, dll, 0)) Sleep(200);

	module->handle = LoadLibraryA(dll);
	if (!module->handle)
	{
		printf("[Loader] Falha ao recarregar: %s\n", dll);
	}

	printf("[Loader] Módulo recarregado com sucesso %s\n", dll);
}

inline void StructaFreeModule(StructaModule module)
{
	if (module->handle == NULL)
		return;

	FreeLibrary(module->handle);
	module->handle = NULL;
}

inline void* StructaLoaderGetFunc(StructaModule module, const char* funcName)
{
	if (!module->handle) return NULL;
	return (void*)GetProcAddress(module->handle, funcName);
}

inline bool StructaCheckVersion(StructaModule module)
{
	time_t newTime = GetFileTimeStamp(module->path);
	if (newTime == 0) 
	{
		printf("[Loader] Erro inesperado, arquivo não encontrado: %s\n", module->path);
		return false;
	}

	if (newTime != module->lastWrite) 
	{
		module->lastWrite = newTime;
		return true;
	}

	return false;
}

inline time_t GetFileTimeStamp(const char* path)
{
	struct stat attr;
	if (stat(path, &attr) != 0) return 0;
	return attr.st_mtime;
}

#endif // STRUCTA_LOADER_H_