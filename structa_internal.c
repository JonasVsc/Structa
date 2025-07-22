#include "structa_internal.h"
#include "structa_core.h"

static StMemory_T g_memory = { 0 };


StResult stInit()
{
	// ALLOC SYSTEM STORAGE
	const size_t total_size = sizeof(StWindow_T);
	g_memory.system_storage = (StWindow_T*)calloc(1, total_size);
	g_memory.size = total_size;

	// INIT SubSystem Pointers
	g_memory.ptr_window = g_memory.system_storage;

	return ST_SUCCESS;
}

void stShutdown()
{
	if (g_memory.system_storage == NULL) return;

	free(g_memory.system_storage);
	g_memory.system_storage = NULL;
}

StWindow structa_internal_window_ptr()
{
	return g_memory.ptr_window;
}

