#include "structa_loader.h"
#include "structa_internal.h"

#include <sys/stat.h>

void StructaLoadModule(StructaModule module, const char* path)
{
	if (path != NULL)
	{
		module->path = path;
	}

	module->lastWrite = GetFileTimeStamp(module->path);

	char dll[256];
	snprintf(dll, sizeof(dll), "_%s_%lld.dll", module->path, module->lastWrite);

	printf("[Loader] Copying  %s to %s\n", module->path, dll);
	while (!CopyFileA(module->path, dll, 0)) Sleep(200);

	module->handle = LoadLibraryA(dll);
	if (!module->handle)
	{
		printf("[Loader] Failed on reloading: %s\n", dll);
	}

	printf("[Loader] Module loaded successfully %s\n", dll);
}

void StructaFreeModule(StructaModule module)
{
	if (module->handle == NULL)
		return;

	FreeLibrary(module->handle);
	module->handle = NULL;
}

void* StructaLoaderGetFunc(StructaModule module, const char* funcName)
{
	if (!module->handle) return NULL;
	return (void*)GetProcAddress(module->handle, funcName);
}

bool StructaCheckVersion(StructaModule module)
{
	time_t newTime = GetFileTimeStamp(module->path);
	if (newTime == 0)
	{
		printf("[Loader] Unexpected Error, file not found: %s\n", module->path);
		return false;
	}

	if (newTime != module->lastWrite)
	{
		module->lastWrite = newTime;
		return true;
	}

	return false;
}

time_t GetFileTimeStamp(const char* path)
{
	struct stat attr;
	if (stat(path, &attr) != 0) return 0;
	return attr.st_mtime;
}